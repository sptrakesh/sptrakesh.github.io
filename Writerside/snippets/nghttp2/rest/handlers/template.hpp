//
// Created by Rakesh on 15/02/2021.
//

#pragma once

#include "common.hpp"
#include "db/repository.hpp"
#include "db/filter/between.hpp"
#include "model/entities.hpp"
#include "model/entitiesquery.hpp"
#include "util/cache.hpp"
#include "util/config.hpp"
#include "validate/validate.hpp"

#include <sstream>

#include <bsoncxx/exception/exception.hpp>
#include <log/NanoLog.hpp>
#include <mongo-service/common/util/bson.hpp>
#include <mongo-service/common/util/date.hpp>

namespace spt::http2::rest::handlers
{
  using std::operator""s;
  using std::operator""sv;
  using model::Model;

  namespace ptemplate
  {
    template <Model M>
    bool canRead( const M& m, const model::JwtToken& token )
    {
      if ( !hasEntityAccess( m.EntityType(), token ) ) return false;

      if ( token.user.role == model::Role::superuser ) return true;
      // implement other rules as appropriate
      return true; // return result of applying rules
    }
  }

  template <Model M, typename AuthFunction>
  auto create( const Request& req,
      std::string_view payload, const std::vector<std::string>& methods,
      AuthFunction&& authfn, bool skipVersion = false ) -> Response
  {
    LOG_INFO << "Handling POST request for " << req.path;

    if ( payload.empty() )
    {
      LOG_WARN << "Request to " << req.path << " did not include payload.";
      return error( 400, "No payload"sv, methods, req.header );
    }

    try
    {
      auto [astatus, jwt] = authorise( req );
      if ( astatus != 200 || !jwt ) return error( astatus, methods, req.header );
      if ( !authfn( M::EntityType(), *jwt ) ) return error( 401, methods, req.header );

      auto m = M{ payload };

      if ( m.id != model::DEFAULT_OID )
      {
        LOG_WARN << "Create entity payload included id. " << payload;
        return error( 400, "Cannot specify id"sv, methods, req.header );
      }

      auto [vstatus, vmesg, vtime] = validate::validate( m, *jwt );
      if ( vstatus != 200 )
      {
        return error( vstatus, vmesg, methods, req.header );
      }

      m.id = bsoncxx::oid{};

      auto [status, time] = db::create( m, skipVersion );
      if ( status != 200 )
      {
        return error( status, "Error creating entity"sv, methods, req.header );
      }

      std::stringstream ss;
      ss << m;
      auto [out, compressed] = shouldCompress( req ) ? compress( ss ) : Output{ ss.str(), false };

      LOG_INFO << "Writing response for " << req.path;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      resp.body = std::move( out );
      resp.compressed = compressed;
      resp.correlationId = correlationId( req );
      resp.set( methods );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const simdjson::simdjson_error& e )
    {
      LOG_WARN << "JSON parse error processing " << req.path << ". " << e.what();
      return error( 400, "Error parsing payload"sv, methods, req.header );
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what();
      LOG_WARN << util::stacktrace();
      return error( 417, "Error creating entity"sv, methods, req.header );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what();
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". ";
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
  }

  template <Model M, typename AuthFunction>
  auto at( const Request& req, std::string_view timestamp, std::string_view payload, const std::vector<std::string>& methods,
      AuthFunction&& authfn, bool skipVersion = false ) -> Response
  {
    LOG_INFO << "Handling POST request for " << req.path;

    if ( payload.empty() )
    {
      LOG_WARN << "Request to " << req.path << " did not include payload.";
      return error( 400, "No payload"sv, methods, req.header );
    }

    if ( timestamp.empty() )
    {
      LOG_WARN << "Request to " << req.path << " did not include timestamp.";
      return error( 400, "No timestamp"sv, methods, req.header );
    }

    try
    {
      auto ts = spt::util::parseISO8601( timestamp );
      if ( !ts.has_value() )
      {
        LOG_WARN << "Invalid timestamp: " << timestamp;
        return error( 400, "Invalid timestamp"sv, methods, req.header );
      }

      auto [astatus, jwt] = authorise( req );
      if ( astatus != 200 || !jwt ) return error( astatus, methods, req.header );
      if ( !authfn( M::EntityType(), *jwt ) ) return error( 401, methods, req.header );

      auto m = M{ payload };

      if ( m.id != model::DEFAULT_OID )
      {
        LOG_WARN << "Create entity payload included id. " << payload;
        return error( 400, "Cannot specify id"sv, methods, req.header );
      }

      auto [vstatus, vmesg, vtime] = validate::validate( m, *jwt );
      if ( vstatus != 200 )
      {
        return error( vstatus, vmesg, methods, req.header );
      }

      m.id = spt::util::generateId( *ts, bsoncxx::oid{} );
      auto count = 0;
      while ( count < 10 ) // since we are generating id at specified time, ensure there is no clash
      {
        auto [status, time, opt] = db::retrieve<M>( m.id );
        if ( opt )
        {
          m.id = spt::util::generateId( *ts, bsoncxx::oid{} );
          ++count;
        }
        else break;
      }

      m.metadata.created = *ts;

      auto [status, time] = db::create( m, skipVersion );
      if ( status != 200 )
      {
        return error( status, "Error creating entity"sv, methods, req.header );
      }

      std::stringstream ss;
      ss << m;
      auto [out, compressed] = shouldCompress( req ) ? compress( ss ) : Output{ ss.str(), false };

      LOG_INFO << "Writing response for " << req.path;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      resp.body = std::move( out );
      resp.compressed = compressed;
      resp.correlationId = correlationId( req );
      resp.set( methods );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const simdjson::simdjson_error& e )
    {
      LOG_WARN << "JSON parse error processing " << req.path << ". " << e.what();
      return error( 400, "Error parsing payload"sv, methods, req.header );
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what();
      LOG_WARN << util::stacktrace();
      return error( 417, "Error creating entity"sv, methods, req.header );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what();
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". ";
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
  }

  template <Model M, typename AuthFunction>
  auto update( const Request& req, std::string_view payload, std::string_view entityId,
      const std::vector<std::string>& methods, AuthFunction&& authfn ) -> Response
  {
    LOG_INFO << "Handling PUT request for " << req.path;

    if ( payload.empty() )
    {
      LOG_WARN << "Request to " << req.path << " did not include payload.";
      return error( 400, "No payload"sv, methods, req.header );
    }

    try
    {
      auto [astatus, jwt] = authorise( req );
      if ( astatus != 200 || !jwt )
      {
        return error( astatus, methods, req.header );
      }

      if ( !authfn( M::EntityType(), *jwt ) )
      {
        return error( 401, methods, req.header );
      }

      auto m = M{ payload };

      if ( m.id.to_string() != entityId )
      {
        LOG_WARN << "Update entity id " << m.id <<
          " not same as path " << entityId << ". " << spt::util::json::str( m );
        return error( 400, "Incorrect id"sv, methods, req.header );
      }

      auto [vstatus, vmesg, vtime] = validate::validate( m, *jwt );
      if ( vstatus != 200 )
      {
        return error( vstatus, vmesg, methods, req.header );
      }

      std::optional<bsoncxx::oid> restoredFrom = std::nullopt;
      if ( auto iter = req.header.find( "x-spt-restored-from" ); iter != req.header.end() ) restoredFrom = spt::util::parseId( iter->second.value );

      auto [status, time] = db::update( m, false, restoredFrom );
      if ( status != 200 )
      {
        return error( status, "Error updating entity"sv, methods, req.header );
      }

      std::stringstream ss;

      if ( auto p = model::pipeline<M>(); !p.empty() )
      {
        const auto [mstatus, mtime, mopt] = db::retrieve<M>( m.id, m.customer.code );
        if ( mopt ) ss << *mopt;
        else ss << m;
      }
      else ss << m;
      auto [out, compressed] = shouldCompress( req ) ? compress( ss ) : Output{ ss.str(), false };

      LOG_INFO << "Writing response for " << req.path;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      resp.body = std::move( out );
      resp.compressed = compressed;
      resp.correlationId = correlationId( req );
      resp.set( methods );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const simdjson::simdjson_error& e )
    {
      LOG_WARN << "JSON parse error processing " << req.path << ". " << e.what();
      return error( 400, "Error parsing payload"sv, methods, req.header );
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what();
      LOG_WARN << util::stacktrace();
      return error( 417, "Error updating entity"sv, methods, req.header );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what();
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". ";
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
  }

  template <Model M, typename AuthFunction>
  auto get( const Request& req, std::string_view entityId, const std::vector<std::string>& methods, AuthFunction&& authfn ) -> Response
  {
    try
    {
      LOG_INFO << "Handling GET request for " << req.path;

      auto [astatus, jwt] = authorise( req );
      if ( astatus != 200 || !jwt )
      {
        return error( astatus, methods, req.header );
      }

      if ( !authfn( M::EntityType(), *jwt ) )
      {
        return error( 401, methods, req.header );
      }

      auto id = spt::util::parseId( entityId );
      if ( !id )
      {
        LOG_INFO << "Rejecting request for " << req.path << " with invalid id " << entityId;
        return error( 400, methods, req.header );
      }

      auto [mstatus, mtime, m] = db::retrieve<M>( *id );
      if ( mstatus != 200 && mstatus != 404 )
      {
        return error( mstatus, "Error retrieving entity"sv, methods, req.header );
      }

      if ( !m ) return error( 404, methods, req.header );

      if ( !ptemplate::canRead( *m, *jwt ) )
      {
        return error( 403, "User not allowed to view entity"sv, methods, req.header );
      }

      std::stringstream ss;
      ss << *m;
      auto [out, compressed] = shouldCompress( req ) ? compress( ss ) : Output{ ss.str(), false };

      LOG_INFO << "Writing response for " << req.path;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      resp.body = std::move( out );
      resp.compressed = compressed;
      resp.correlationId = correlationId( req );
      resp.set( methods );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what();
      LOG_WARN << util::stacktrace();
      return error( 417, "Error retrieving entity"sv, methods, req.header );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what();
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". ";
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
  }

  template <Model M, typename ValueType, typename AuthFunction>
  auto get( const Request& req, std::string_view property, ValueType value,
      const std::vector<std::string>& methods, AuthFunction&& authfn,
      bool caseInsensitive = false ) -> Response
  {
    try
    {
      LOG_INFO << "Handling GET request for " << req.path;

      auto [astatus, jwt] = authorise( req );
      if ( astatus != 200 || !jwt )
      {
        return error( astatus, methods, req.header );
      }

      if ( !authfn( M::EntityType(), *jwt ) )
      {
        return error( 401, methods, req.header );
      }

      auto [mstatus, mtime, m] = db::retrieve<M,ValueType>( property, value, caseInsensitive );
      if ( mstatus != 200 && mstatus != 404 )
      {
        return error( mstatus, "Error retrieving entity"sv, methods, req.header );
      }

      if ( !m ) return error( 404, methods, req.header );

      if ( !ptemplate::canRead( *m, *jwt ) )
      {
        return error( 403, "User not allowed to view entity"sv, methods, req.header );
      }

      std::stringstream ss;
      ss << *m;
      auto [out, compressed] = shouldCompress( req ) ? compress( ss ) : Output{ ss.str(), false };

      LOG_INFO << "Writing response for " << req.path;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      resp.body = std::move( out );
      resp.compressed = compressed;
      resp.correlationId = correlationId( req );
      resp.set( methods );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what();
      LOG_WARN << util::stacktrace();
      return error( 417, "Error retrieving entity"sv, methods, req.header );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what();
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". ";
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
  }

  template <Model M, typename AuthFunction>
  auto retrieveAll( const Request& req,
      const std::vector<std::string>& methods, AuthFunction&& authfn ) -> Response
  {
    LOG_INFO << "Handling GET request for " << req.path;

    try
    {
      auto [qstatus, eq] = parseQuery( req );
      if ( qstatus != 200 || !eq ) return error( 400, "Invalid query"sv, methods, req.header );

      auto [astatus, jwt] = authorise( req );
      if ( astatus != 200 || !jwt ) return error( astatus, methods, req.header );

      if ( !authfn( M::EntityType(), *jwt ) ) return error( 401, methods, req.header );

      auto query = bsoncxx::builder::stream::document{};
      if ( eq->after )
      {
        auto oid = spt::util::parseId( *eq->after );
        if ( !oid )
        {
          LOG_INFO << "Rejecting request for " << req.path << " with invalid after " << *eq->after;
          return error( 400, methods, req.header );
        }

        query <<
          "_id" <<
            bsoncxx::builder::stream::open_document <<
              ( eq->descending ? "$lt" : "$gt" ) << *oid <<
            bsoncxx::builder::stream::close_document;
      }

      auto [mstatus, mtime, m] = db::query<M>( query << bsoncxx::builder::stream::finalize, *eq );
      if ( mstatus != 200 && mstatus != 404 )
      {
        return error( mstatus, "Error retrieving entity"sv, methods, req.header );
      }

      if ( !m ) return error( 404, methods, req.header );

      std::stringstream ss;
      ss << *m;
      auto [out, compressed] = shouldCompress( req ) ? compress( ss ) : Output{ ss.str(), false };

      LOG_INFO << "Writing response for " << req.path;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      resp.body = std::move( out );
      resp.correlationId = correlationId( req );
      resp.compressed = compressed;
      resp.set( methods );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what();
      LOG_WARN << util::stacktrace();
      return error( 417, "Error retrieving entity"sv, methods, req.header );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what();
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". ";
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
  }

  template <Model M, typename ValueType, typename AuthFunction>
  auto retrieveAll( const Request& req,
      std::string_view property, ValueType value,
      const std::vector<std::string>& methods, AuthFunction&& authfn ) -> Response
  {
    LOG_INFO << "Handling GET request for " << req.path;

    try
    {
      auto [qstatus, eq] = parseQuery( req );
      if ( qstatus != 200 || !eq ) return error( 400, "Invalid query"sv, methods, req.header );

      auto [astatus, jwt] = authorise( req );
      if ( astatus != 200 || !jwt ) return error( astatus, methods, req.header );

      if ( !authfn( M::EntityType(), *jwt ) ) return error( 401, methods, req.header );

      auto query = bsoncxx::builder::stream::document{};
      query << property << value;

      if ( eq->after )
      {
        auto oid = spt::util::parseId( *eq->after );
        if ( !oid )
        {
          LOG_INFO << "Rejecting request for " << req.path << " with invalid after " << *eq->after;
          return error( 400, methods, req.header );
        }

        query <<
          "_id" <<
          bsoncxx::builder::stream::open_document <<
            ( eq->descending ? "$lt" : "$gt" ) << *oid <<
          bsoncxx::builder::stream::close_document;
      }
      auto [mstatus, mtime, m] = db::query<M>( query << bsoncxx::builder::stream::finalize, *eq );
      if ( mstatus != 200 && mstatus != 404 )
      {
        return error( mstatus, "Error retrieving entity"sv, methods, req.header );
      }

      if ( !m ) return error( 404, methods, req.header );

      std::stringstream ss;
      ss << *m;
      auto [out, compressed] = shouldCompress( req ) ? compress( ss ) : Output{ ss.str(), false };

      LOG_INFO << "Writing response for " << req.path;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      resp.body = std::move( out );
      resp.correlationId = correlationId( req );
      resp.compressed = compressed;
      resp.set( methods );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what();
      LOG_WARN << util::stacktrace();
      return error( 417, "Error retrieving entity"sv, methods, req.header );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what();
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". ";
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
  }

  template <Model M, typename AuthFunction>
  auto between( const Request& req,
      std::string_view property,
      std::chrono::time_point<std::chrono::system_clock> start,
      std::chrono::time_point<std::chrono::system_clock> end,
      const std::vector<std::string>& methods,
      AuthFunction&& authfn ) -> Response
  {
    LOG_INFO << "Handling GET request for " << req.path;

    try
    {
      auto [qstatus, eq] = parseQuery( req );
      if ( qstatus != 200 || !eq )
      {
        return error( 400, "Invalid query"sv, methods, req.header );
      }

      auto [astatus, jwt] = authorise( req );
      if ( astatus != 200 || !jwt ) return error( astatus, methods, req.header );

      if ( !authfn( M::EntityType(), *jwt ) ) return error( 401, methods, req.header );

      auto filter = db::filter::Between{};
      filter.field = property;
      filter.from = start;
      filter.to = end;
      filter.descending = eq->descending;

      if ( eq->after )
      {
        const auto a = spt::util::parseISO8601( *eq->after );
        if ( !a.has_value() )
        {
          LOG_INFO << "Rejecting request for " << req.path << " with invalid after " << *eq->after;
          return error( 400, a.error(), methods, req.header );
        }

        filter.after = a.value();
      }

      auto [mstatus, mtime, m] = db::between<M>( std::move( filter ), *eq );
      if ( mstatus != 200 && mstatus != 404 )
      {
        return error( mstatus, "Error retrieving entities"sv, methods, req.header );
      }

      if ( !m ) return error( 404, methods, req.header );

      std::stringstream ss;
      ss << *m;
      auto [out, compressed] = shouldCompress( req ) ? compress( ss ) : Output{ ss.str(), false };

      LOG_INFO << "Writing response for " << req.path;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      resp.body = std::move( out );
      resp.correlationId = correlationId( req );
      resp.compressed = compressed;
      resp.set( methods );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what();
      LOG_WARN << util::stacktrace();
      return error( 417, "Error retrieving entity"sv, methods, req.header );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what();
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". ";
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
  }

  template <Model M, typename AuthFunction>
  auto refcounts( const Request& req,
      std::string_view entityId, const std::vector<std::string>& methods,
      AuthFunction&& authfn ) -> Response
  {
    LOG_INFO << "Handling GET request for " << req.path;

    try
    {
      auto [astatus, jwt] = authorise( req );
      if ( astatus != 200 || !jwt ) return error( astatus, methods, req.header );

      if ( !authfn( M::EntityType(), *jwt ) ) return error( 401, methods, req.header );

      auto id = spt::util::parseId( entityId );
      if ( !id )
      {
        LOG_INFO << "Rejecting request for " << req.path << " with invalid id " << entityId;
        return error( 400, methods, req.header );
      }

      auto [mstatus, mtime, m] = db::refcounts<M>( *id );
      if ( mstatus != 200 && mstatus != 404 )
      {
        return error( mstatus, "Error counting references to entity"sv, methods, req.header );
      }

      if ( mstatus == 404 || !m ) return error( 404, methods, req.header );

      std::stringstream ss;
      ss << *m;
      auto [out, compressed] = shouldCompress( req ) ? compress( ss ) : Output{ ss.str(), false };

      LOG_INFO << "Writing response for " << req.path;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      resp.body = std::move( out );
      resp.correlationId = correlationId( req );
      resp.compressed = compressed;
      resp.set( methods );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what();
      LOG_WARN << util::stacktrace();
      return error( 417, "Error deleting entity"sv, methods, req.header );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what();
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". ";
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
  }

  template <Model M, typename AuthFunction>
  auto remove( const Request& req, std::string_view entityId,
      const std::vector<std::string>& methods, AuthFunction&& authfn ) -> Response
  {
    LOG_INFO << "Handling DELETE request for " << req.path;

    try
    {
      auto [astatus, jwt] = authorise( req );
      if ( astatus != 200 || !jwt ) return error( astatus, methods, req.header );

      if ( !authfn( M::EntityType(), *jwt ) ) return error( 401, methods, req.header );

      auto id = spt::util::parseId( entityId );
      if ( !id )
      {
        LOG_INFO << "Rejecting request for " << req.path << " with invalid id " << entityId;
        return error( 400, methods, req.header );
      }

      auto [mstatus, mtime] = db::remove<M>( *id );
      if ( mstatus != 200 && mstatus != 404 ) return error( mstatus, "Error deleting entity"sv, methods, req.header );

      if ( mstatus == 404 ) return error( mstatus, methods, req.header );

      LOG_INFO << "Writing response for " << req.path;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      resp.correlationId = correlationId( req );
      resp.set( methods );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what();
      LOG_WARN << util::stacktrace();
      return error( 417, "Error deleting entity"sv, methods, req.header );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what();
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". ";
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
  }

  template <Model M, typename AuthFunction>
  auto versionHistorySummary( const Request& req, std::string_view entityId,
      const std::vector<std::string>& methods, AuthFunction&& authfn ) -> Response
  {
    LOG_INFO << "Handling GET request for " << req.path;

    try
    {
      auto [astatus, jwt] = authorise( req );
      if ( astatus != 200 || !jwt ) return error( astatus, methods, req.header );

      if ( !authfn( M::EntityType(), *jwt ) ) return error( 401, methods, req.header );

      auto id = spt::util::parseId( entityId );
      if ( !id )
      {
        LOG_INFO << "Rejecting request for " << req.path << " with invalid id " << entityId;
        return error( 400, methods, req.header );
      }

      auto [mstatus, mtime, m] = db::versionHistorySummary<M>( *id );
      if ( mstatus != 200 && mstatus != 404 )
      {
        return error( mstatus, "Error retrieving entity history summaries"sv, methods, req.header );
      }

      if ( !m ) return error( 404, methods, req.header );

      std::stringstream ss;
      ss << *m;
      auto [out, compressed] = shouldCompress( req ) ? compress( ss ) : Output{ ss.str(), false };

      LOG_INFO << "Writing response for " << req.path;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      resp.body = std::move( out );
      resp.correlationId = correlationId( req );
      resp.compressed = compressed;
      resp.set( methods );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what();
      LOG_WARN << util::stacktrace();
      return error( 417, "Error retrieving entity history summaries"sv, methods, req.header );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what();
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". ";
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
  }

  template <Model M, typename AuthFunction>
  auto versionHistoryDocument( const Request& req,
      std::string_view historyId, const std::vector<std::string>& methods,
      AuthFunction authfn ) -> Response
  {
    LOG_INFO << "Handling GET request for " << req.path;

    try
    {
      auto [astatus, jwt] = authorise( req );
      if ( astatus != 200 || !jwt ) return error( astatus, methods, req.header );

      if ( !authfn( M::EntityType(), *jwt ) ) return error( 401, methods, req.header );

      auto id = spt::util::parseId( historyId );
      if ( !id )
      {
        LOG_INFO << "Rejecting request for " << req.path << " with invalid id " << historyId;
        return error( 400, methods, req.header );
      }

      auto [mstatus, mtime, m] = db::versionHistoryDocument<M>( *id );
      if ( mstatus != 200 && mstatus != 404 )
      {
        return error( mstatus, "Error retrieving entity history document"sv, methods, req.header );
      }

      if ( !m ) return error( 404, methods, req.header );

      std::stringstream ss;
      ss << *m;
      auto [out, compressed] = shouldCompress( req ) ? compress( ss ) : Output{ ss.str(), false };

      LOG_INFO << "Writing response for " << req.path;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      resp.body = std::move( out );
      resp.correlationId = correlationId( req );
      resp.compressed = compressed;
      resp.set( methods );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what();
      LOG_WARN << util::stacktrace();
      return error( 417, "Error retrieving entity history summaries"sv, methods, req.header );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what();
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". ";
      LOG_WARN << util::stacktrace();
      return error( 500, "Internal server error"sv, methods, req.header );
    }
  }
}

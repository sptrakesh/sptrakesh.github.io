//
// Created by Rakesh on 15/02/2021.
//

#pragma once

#include "common.hpp"
#include "handlers.hpp"
#include "db/repository.hpp"
#include "db/storage.hpp"
#include "db/filter/between.hpp"
#include "model/entities.hpp"
#include "model/entitiesquery.hpp"
#include "model/paginate.hpp"
#include "util/config.hpp"
#include "validate/validate.hpp"

#include <sstream>

#include <bsoncxx/exception/exception.hpp>
#include <log/NanoLog.hpp>
#include <http2/framework/compress.hpp>
#include <mongo-service/common/util/bson.hpp>
#include <mongo-service/common/util/date.hpp>

namespace spt::http
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
      if ( !m.customer ) return false;

      return m.customer.code == token.user.customerCode;
    }
  }

  template <Model M, typename AuthFunction>
  auto create( const spt::http2::framework::Request& req, std::string_view payload,
      std::span<const std::string> methods, spt::ilp::APMRecord& apm,
      AuthFunction&& authfn, bool skipVersion = false ) -> Response
  {
    LOG_INFO << "Handling POST request for " << req.path << ". APM id: " << apm.id;

    auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    cp.values.try_emplace( model::ilp::name::APM_NOTE_VALUE, std::format( "Create entity {}", M::EntityType() ) );
    DEFER( spt::ilp::setDuration( cp ) );

    if ( payload.empty() )
    {
      LOG_WARN << "Request to " << req.path << " did not include payload. APM id: " << apm.id;
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Empty payload" );
      return error( 400, "No payload"sv, methods, req.header, apm );
    }

    try
    {
      auto idx = apm.processes.size();
      auto [astatus, jwt] = authorise( req, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );

      if ( astatus != 200 || !jwt )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Authorisation failed" );
        return error( astatus, methods, req.header, apm );
      }

      if ( !authfn( M::EntityType(), *jwt ) )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "User role not authorised" );
        return error( 401, methods, req.header, apm );
      }

      auto m = M{ payload };

      if ( m.id != model::DEFAULT_OID )
      {
        LOG_WARN << "Create entity payload included id. " << payload << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Id specified" );
        return error( 400, "Cannot specify id"sv, methods, req.header, apm );
      }

      idx = apm.processes.size();
      auto [vstatus, vmesg] = validate::validate( m, *jwt, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( vstatus != 200 )
      {
        LOG_WARN << "Validation failed. " << vmesg << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, model::ilp::value::APM_VALIDATION_ERROR );
        return error( vstatus, vmesg, methods, req.header, apm );
      }

      if ( m.id == model::DEFAULT_OID ) m.id = bsoncxx::oid{};
      cp.values.try_emplace( "entity_id", m.id.to_string() );

      idx = apm.processes.size();
      auto status = db::create( m, apm, skipVersion );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( status != 200 )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Database error" );
        return error( status, "Error creating entity"sv, methods, req.header, apm );
      }

      LOG_INFO << "Writing response for " << req.path << ". APM id: " << apm.id;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      idx = apm.processes.size();
      output( req, resp, m, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      resp.correlationId = correlationId( req );
      resp.set( methods, Response::origins() );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const simdjson::simdjson_error& e )
    {
      LOG_WARN << "JSON parse error processing " << req.path << ". " << e.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, e, "JSON exception" );
      return error( 400, "Error parsing payload"sv, methods, req.header, apm );
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, b, "BSON exception" );
      return error( 417, "Error creating entity"sv, methods, req.header, apm );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return error( 500, "Internal server error"sv, methods, req.header, apm );
  }

  template <Model M, typename AuthFunction>
  auto at( const spt::http2::framework::Request& req, std::string_view timestamp, std::string_view payload,
      std::span<const std::string> methods, spt::ilp::APMRecord& apm,
      AuthFunction&& authfn, bool skipVersion = false ) -> Response
  {
    LOG_INFO << "Handling POST request for " << req.path << ". APM id: " << apm.id;

    auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    cp.values.try_emplace( model::ilp::name::APM_NOTE_VALUE, std::format( "Create entity {} at time", M::EntityType() ) );
    DEFER( spt::ilp::setDuration( cp ) );

    if ( payload.empty() )
    {
      LOG_WARN << "Request to " << req.path << " did not include payload. APM id: " << apm.id;
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Empty payload" );
      return error( 400, "No payload"sv, methods, req.header, apm );
    }

    if ( timestamp.empty() )
    {
      LOG_WARN << "Request to " << req.path << " did not include timestamp. APM id: " << apm.id;
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Timestamp not in payload" );
      return error( 400, "No timestamp"sv, methods, req.header, apm );
    }

    try
    {
      spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Step );
      apm.processes.back().values.try_emplace( model::ilp::name::APM_STEP_PROCESS, "Parse timestamp" );
      spt::ilp::addCurrentFunction( apm.processes.back() );

      auto ts = spt::util::parseISO8601( timestamp );
      spt::ilp::setDuration( apm.processes.back() );
      if ( !ts.has_value() )
      {
        LOG_WARN << "Invalid timestamp: " << timestamp << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid timestamp in payload" );
        return error( 400, "Invalid timestamp"sv, methods, req.header, apm );
      }

      auto idx = apm.processes.size();
      auto [astatus, jwt] = authorise( req, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );

      if ( astatus != 200 || !jwt )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid bearer token" );
        return error( astatus, methods, req.header, apm );
      }
      if ( !authfn( M::EntityType(), *jwt ) )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "User role not authorised" );
        return error( 401, methods, req.header, apm );
      }

      auto m = M{ payload };

      if ( m.id != model::DEFAULT_OID )
      {
        LOG_WARN << "Create entity payload included id. " << payload << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Payload included id" );
        return error( 400, "Cannot specify id"sv, methods, req.header, apm );
      }

      idx = apm.processes.size();
      auto [vstatus, vmesg] = validate::validate( m, *jwt, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( vstatus != 200 )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Validation failed" );
        return error( vstatus, vmesg, methods, req.header, apm );
      }

      {
        spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Step );
        apm.processes.back().values.try_emplace( model::ilp::name::APM_STEP_PROCESS, "Generate id" );
        spt::ilp::addCurrentFunction( apm.processes.back() );
        m.id = spt::util::generateId( *ts, bsoncxx::oid{} );
        auto count = 0;
        while ( count < 10 )
        {
          if ( const auto [_, opt] = db::retrieve<M>( m.id, ""sv ); opt )
          {
            m.id = spt::util::generateId( *ts, bsoncxx::oid{} );
            ++count;
          }
          else break;
        }
        spt::ilp::setDuration( apm.processes.back() );
      }

      cp.values.try_emplace( "entity_id", m.id.to_string() );
      m.metadata.created = *ts;

      idx = apm.processes.size();
      auto status = db::create( m, apm, skipVersion );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( status != 200 )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Error creating entity" );
        return error( status, "Error creating entity"sv, methods, req.header, apm );
      }

      LOG_INFO << "Writing response for " << req.path << ". APM id: " << apm.id;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      idx = apm.processes.size();
      output( req, resp, m, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      resp.correlationId = correlationId( req );
      resp.set( methods, Response::origins() );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const simdjson::simdjson_error& e )
    {
      LOG_WARN << "JSON parse error processing " << req.path << ". " << e.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, e, "JSON exception" );
      return error( 400, "Error parsing payload"sv, methods, req.header, apm );
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, b, "BSON exception" );
      return error( 417, "Error creating entity"sv, methods, req.header, apm );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return error( 500, "Internal server error"sv, methods, req.header, apm );
  }

  template <Model M, typename AuthFunction>
  auto update( const spt::http2::framework::Request& req, std::string_view payload, std::string_view entityId,
      std::span<const std::string> methods, spt::ilp::APMRecord& apm, AuthFunction&& authfn ) -> Response
  {
    LOG_INFO << "Handling PUT request for " << req.path << ". APM id: " << apm.id;

    auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    cp.values.try_emplace( model::ilp::name::APM_NOTE_VALUE, std::format( "Update entity {}", M::EntityType() ) );
    DEFER( spt::ilp::setDuration( cp ) );
    cp.values.try_emplace( "entity_id", std::string{ entityId } );

    if ( payload.empty() )
    {
      LOG_WARN << "Request to " << req.path << " did not include payload. APM id: " << apm.id;
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Empty payload" );
      return error( 400, "No payload"sv, methods, req.header, apm );
    }

    try
    {
      auto idx = apm.processes.size();
      auto [astatus, jwt] = authorise( req, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( astatus != 200 || !jwt )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid bearer token" );
        return error( astatus, methods, req.header, apm );
      }

      if ( !authfn( M::EntityType(), *jwt ) )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "User role not authorised" );
        return error( 401, methods, req.header, apm );
      }

      auto m = M{ payload };

      if ( m.id.to_string() != entityId )
      {
        LOG_WARN << "Update entity id " << m.id <<
          " not same as path " << entityId << ". " << spt::util::json::str( m ) << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Mis-matching entity id" );
        return error( 400, "Incorrect id"sv, methods, req.header, apm );
      }

      idx = apm.processes.size();
      const auto [vstatus, vmesg] = validate::validate( m, *jwt, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( vstatus != 200 )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Validation failed" );
        return error( vstatus, vmesg, methods, req.header, apm );
      }

      std::optional<bsoncxx::oid> restoredFrom = std::nullopt;
      if ( auto iter = req.header.find( "x-wp-restored-from" ); iter != req.header.end() ) restoredFrom = spt::util::parseId( iter->second.value );

      idx = apm.processes.size();
      const auto status = db::update( m,
          jwt->user.role == model::Role::superuser ? ""sv : jwt->user.customerCode,
          apm, false, restoredFrom );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( status != 200 )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Database error" );
        return error( status, "Error updating entity"sv, methods, req.header, apm );
      }

      if ( const auto p = model::pipeline<M>(); !p.empty() )
      {
        idx = apm.processes.size();
        auto [mstatus, mopt] = db::retrieve<M>( m.id, m.customer.code, apm );
        spt::ilp::addCurrentFunction( apm.processes[idx] );
        if ( mopt ) m = std::move( *mopt );
      }

      LOG_INFO << "Writing response for " << req.path << ". APM id: " << apm.id;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      idx = apm.processes.size();
      output( req, resp, m, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      resp.correlationId = correlationId( req );
      resp.set( methods, Response::origins() );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const simdjson::simdjson_error& e )
    {
      LOG_WARN << "JSON parse error processing " << req.path << ". " << e.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, e, "JSON exception" );
      return error( 400, "Error parsing payload"sv, methods, req.header, apm );
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, b, "BSON exception" );
      return error( 417, "Error creating entity"sv, methods, req.header, apm );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return error( 500, "Internal server error"sv, methods, req.header, apm );
  }

  template <Model M, typename AuthFunction>
  auto get( const spt::http2::framework::Request& req, std::string_view entityId, std::span<const std::string> methods,
      spt::ilp::APMRecord& apm, AuthFunction&& authfn ) -> Response
  {
    LOG_INFO << "Handling GET request for " << req.path << ". APM id: " << apm.id;

    auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    DEFER( spt::ilp::setDuration( cp ) );

    try
    {
      auto idx = apm.processes.size();
      auto [astatus, jwt] = authorise( req, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( astatus != 200 || !jwt )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid bearer token" );
        return error( astatus, methods, req.header, apm );
      }

      if ( !authfn( M::EntityType(), *jwt ) )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "User role not authorised" );
        return error( 401, methods, req.header, apm );
      }

      const auto id = spt::util::parseId( entityId );
      if ( !id )
      {
        LOG_INFO << "Rejecting request for " << req.path << " with invalid id " << entityId << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid id" );
        return error( 400, methods, req.header, apm );
      }

      idx = apm.processes.size();
      auto [mstatus, m] = db::retrieve<M>( *id,
          jwt->user.role == model::Role::superuser ? ""sv : jwt->user.customerCode,
          apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( mstatus != 200 && mstatus != 404 )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Database error" );
        return error( mstatus, "Error retrieving entity"sv, methods, req.header, apm );
      }

      if ( !m )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Entity not found" );
        return error( 404, methods, req.header, apm );
      }

      if ( !ptemplate::canRead( *m, *jwt ) )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "User cannot access other customer" );
        return error( 403, "User not allowed to view entity belonging to other customer"sv, methods, req.header, apm );
      }

      LOG_INFO << "Writing response for " << req.path << ". APM id: " << apm.id;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      idx = apm.processes.size();
      output( req, resp, *m, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      resp.correlationId = correlationId( req );
      resp.set( methods, Response::origins() );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, b, "BSON exception" );
      return error( 417, "Error creating entity"sv, methods, req.header, apm );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return error( 500, "Internal server error"sv, methods, req.header, apm );
  }

  template <Model M, typename ValueType, typename AuthFunction>
  auto get( const spt::http2::framework::Request& req,
      std::string_view property, ValueType value,
      std::span<const std::string> methods, spt::ilp::APMRecord& apm,
      AuthFunction&& authfn, bool caseInsensitive = false ) -> Response
  {
    LOG_INFO << "Handling GET request for " << req.path << ". APM id: " << apm.id;

    auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    cp.values.try_emplace( model::ilp::name::APM_NOTE_VALUE, std::format( "Retrieve entity {} by property {}", M::EntityType(), property ) );
    DEFER( spt::ilp::setDuration( cp ) );

    try
    {
      auto idx = apm.processes.size();
      auto [astatus, jwt] = authorise( req, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( astatus != 200 || !jwt )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid bearer token" );
        return error( astatus, methods, req.header, apm );
      }

      if ( !authfn( M::EntityType(), *jwt ) )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "User role not authorised" );
        return error( 401, methods, req.header, apm );
      }

      idx = apm.processes.size();
      auto [mstatus, m] = db::retrieve<M,ValueType>( property, value,
          jwt->user.role == model::Role::superuser ? ""sv : jwt->user.customerCode, apm, caseInsensitive );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( mstatus != 200 && mstatus != 404 )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Error retrieving entity" );
        return error( mstatus, "Error retrieving entity"sv, methods, req.header, apm );
      }

      if ( !m )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Database error" );
        return error( 404, methods, req.header, apm );
      }

      if ( !ptemplate::canRead( *m, *jwt ) )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "User cannot access other customer" );
        return error( 403, "User not allowed to view entity belonging to other customer"sv, methods, req.header, apm );
      }

      LOG_INFO << "Writing response for " << req.path << ". APM id: " << apm.id;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      idx = apm.processes.size();
      output( req, resp, *m, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      resp.correlationId = correlationId( req );
      resp.set( methods, Response::origins() );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, b, "BSON exception" );
      return error( 417, "Error creating entity"sv, methods, req.header, apm );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return error( 500, "Internal server error"sv, methods, req.header, apm );
  }

  template <Model M, typename ValueType>
  auto getForCustomer( const spt::http2::framework::Request& req,
      std::string_view property, ValueType value,
      std::string_view customerCode, std::span<const std::string>& methods, spt::ilp::APMRecord& apm ) -> Response
  {
    LOG_INFO << "Handling GET request for " << req.path << ". APM id: " << apm.id;

    auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    cp.values.try_emplace( model::ilp::name::APM_NOTE_VALUE, std::format( "Retrieve entity {} by property {}", M::EntityType(), property ) );
    DEFER( spt::ilp::setDuration( cp ) );

    try
    {
      auto idx = apm.processes.size();
      auto [astatus, jwt] = authorise( req, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( astatus != 200 || !jwt )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid bearer token" );
        return error( astatus, methods, req.header, apm );
      }

      if ( !superuserRole( M::EntityType(), *jwt ) )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "User role not authorised" );
        return error( 401, methods, req.header, apm );
      }

      idx = apm.processes.size();
      auto [mstatus, m] = db::retrieve<M,ValueType>(
          property, value, std::string{ customerCode.data(), customerCode.size() }, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( mstatus != 200 && mstatus != 404 )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Error retrieving entity" );
        return error( mstatus, "Error retrieving entity"sv, methods, req.header, apm );
      }

      if ( !m )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Database error" );
        return error( mstatus, "Error retrieving entity"sv, methods, req.header, apm );
      }

      LOG_INFO << "Writing response for " << req.path << ". APM id: " << apm.id;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      idx = apm.processes.size();
      output( req, resp, *m, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      resp.correlationId = correlationId( req );
      resp.set( methods, Response::origins() );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, b, "BSON exception" );
      return error( 417, "Error creating entity"sv, methods, req.header, apm );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return error( 500, "Internal server error"sv, methods, req.header, apm );
  }

  template <Model M, typename AuthFunction>
  auto retrieveAll( const spt::http2::framework::Request& req, std::span<const std::string> methods,
      spt::ilp::APMRecord& apm, AuthFunction&& authfn ) -> Response
  {
    LOG_INFO << "Handling GET request for " << req.path << ". APM id: " << apm.id;

    auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    cp.values.try_emplace( model::ilp::name::APM_NOTE_VALUE, std::format( "List entity {}", M::EntityType() ) );
    DEFER( spt::ilp::setDuration( cp ) );

    try
    {
      auto idx = apm.processes.size();
      auto [qstatus, eq] = parseQuery( req, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( qstatus != 200 || !eq )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid query" );
        return error( 400, "Invalid query"sv, methods, req.header, apm );
      }

      idx = apm.processes.size();
      auto [astatus, jwt] = authorise( req, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( astatus != 200 || !jwt )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid bearer token" );
        return error( astatus, methods, req.header, apm );
      }

      if ( !authfn( M::EntityType(), *jwt ) )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "User role not authorised" );
        return error( 401, methods, req.header, apm );
      }

      auto query = bsoncxx::builder::stream::document{};
      if ( jwt->user.role != model::Role::superuser )
      {
        query << "customer.code" << jwt->user.customerCode;
      }

      if ( eq->after )
      {
        auto oid = spt::util::parseId( *eq->after );
        if ( !oid )
        {
          LOG_INFO << "Rejecting request for " << req.path << " with invalid after " << *eq->after << ". APM id: " << apm.id;
          cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid after parameter" );
          return error( 400, methods, req.header, apm );
        }

        query <<
          "_id" <<
            bsoncxx::builder::stream::open_document <<
              ( eq->descending ? "$lt" : "$gt" ) << *oid <<
            bsoncxx::builder::stream::close_document;
      }

      idx = apm.processes.size();
      auto [mstatus, m] = db::query<M>( query << bsoncxx::builder::stream::finalize, *eq, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( mstatus != 200 && mstatus != 404 )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Error retrieving entity" );
        return error( mstatus, "Error retrieving entity"sv, methods, req.header, apm );
      }

      if ( !m )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Database error" );
        return error( 404, methods, req.header, apm );
      }

      LOG_INFO << "Writing response for " << req.path << ". APM id: " << apm.id;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      idx = apm.processes.size();
      output( req, resp, *m, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      resp.correlationId = correlationId( req );
      resp.set( methods, Response::origins() );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, b, "BSON exception" );
      return error( 417, "Error creating entity"sv, methods, req.header, apm );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return error( 500, "Internal server error"sv, methods, req.header, apm );
  }

  template <Model M>
  auto retrieveAll( const spt::http2::framework::Request& req, std::string_view customerCode,
      std::span<const std::string> methods, spt::ilp::APMRecord& apm ) -> Response
  {
    LOG_INFO << "Handling GET request for " << req.path << ". APM id: " << apm.id;

    auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    cp.values.try_emplace( model::ilp::name::APM_NOTE_VALUE, std::format( "List entity {} for customer {}", M::EntityType(), customerCode ) );
    DEFER( spt::ilp::setDuration( cp ) );

    try
    {
      auto idx = apm.processes.size();
      auto [qstatus, eq] = parseQuery( req, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( qstatus != 200 || !eq )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid query" );
        return error( 400, "Invalid query"sv, methods, req.header, apm );
      }

      idx = apm.processes.size();
      auto [astatus, jwt] = authorise( req, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( astatus != 200 || !jwt )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid bearer token" );
        return error( astatus, methods, req.header, apm );
      }

      if ( !superuserRole( M::EntityType(), *jwt ) )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "User role not authorised" );
        return error( 401, methods, req.header, apm );
      }

      auto query = bsoncxx::builder::stream::document{};
      query << "customer.code" << customerCode;

      if ( eq->after )
      {
        auto oid = spt::util::parseId( *eq->after );
        if ( !oid )
        {
          LOG_INFO << "Rejecting request for " << req.path << " with invalid after " << *eq->after << ". APM id: " << apm.id;
          cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid after parameter" );
          return error( 400, methods, req.header, apm );
        }

        query <<
          "_id" <<
            bsoncxx::builder::stream::open_document <<
              ( eq->descending ? "$lt" : "$gt" ) << *oid <<
            bsoncxx::builder::stream::close_document;
      }

      idx = apm.processes.size();
      auto [mstatus, m] = db::query<M>( query << bsoncxx::builder::stream::finalize, *eq, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( mstatus != 200 && mstatus != 404 )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Error retrieving entity" );
        return error( mstatus, "Error retrieving entity"sv, methods, req.header, apm );
      }

      if ( !m )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Database error" );
        return error( 404, methods, req.header, apm );
      }

      LOG_INFO << "Writing response for " << req.path << ". APM id: " << apm.id;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      idx = apm.processes.size();
      output( req, resp, *m, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      resp.correlationId = correlationId( req );
      resp.set( methods, Response::origins() );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, b, "BSON exception" );
      return error( 417, "Error creating entity"sv, methods, req.header, apm );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return error( 500, "Internal server error"sv, methods, req.header, apm );
  }

  template <Model M, typename ValueType, typename AuthFunction>
  auto retrieveAll( const spt::http2::framework::Request& req,
      std::string_view property, ValueType value,
      std::span<const std::string> methods, spt::ilp::APMRecord& apm, AuthFunction&& authfn ) -> Response
  {
    LOG_INFO << "Handling GET request for " << req.path << ". APM id: " << apm.id;

    auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    cp.values.try_emplace( model::ilp::name::APM_NOTE_VALUE, std::format( "List entity {} by property {}", M::EntityType(), property ) );
    DEFER( spt::ilp::setDuration( cp ) );

    try
    {
      auto idx = apm.processes.size();
      auto [qstatus, eq] = parseQuery( req, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( qstatus != 200 || !eq )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid query" );
        return error( 400, "Invalid query"sv, methods, req.header, apm );
      }

      idx = apm.processes.size();
      auto [astatus, jwt] = authorise( req, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( astatus != 200 || !jwt )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid bearer token" );
        return error( astatus, methods, req.header, apm );
      }

      if ( !authfn( M::EntityType(), *jwt ) )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "User role not authorised" );
        return error( 401, methods, req.header, apm );
      }

      auto query = bsoncxx::builder::stream::document{};
      query << property << value;

      if ( jwt->user.role != model::Role::superuser )
      {
        query << "customer.code" << jwt->user.customerCode;
      }

      if ( eq->after )
      {
        auto oid = spt::util::parseId( *eq->after );
        if ( !oid )
        {
          LOG_INFO << "Rejecting request for " << req.path << " with invalid after " << *eq->after << ". APM id: " << apm.id;
          cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid after parameter" );
          return error( 400, methods, req.header, apm );
        }

        query <<
          "_id" <<
          bsoncxx::builder::stream::open_document <<
            ( eq->descending ? "$lt" : "$gt" ) << *oid <<
          bsoncxx::builder::stream::close_document;
      }

      idx = apm.processes.size();
      auto [mstatus, m] = db::query<M>( query << bsoncxx::builder::stream::finalize, *eq, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( mstatus != 200 && mstatus != 404 )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Error retrieving entity" );
        return error( mstatus, "Error retrieving entity"sv, methods, req.header, apm );
      }

      if ( !m )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Database error" );
        return error( 404, methods, req.header, apm );
      }

      LOG_INFO << "Writing response for " << req.path << ". APM id: " << apm.id;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      idx = apm.processes.size();
      output( req, resp, *m, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      resp.correlationId = correlationId( req );
      resp.set( methods, Response::origins() );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, b, "BSON exception" );
      return error( 417, "Error creating entity"sv, methods, req.header, apm );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return error( 500, "Internal server error"sv, methods, req.header, apm );
  }

  template <Model M, typename AuthFunction>
  auto byReference( const spt::http2::framework::Request& req, bsoncxx::oid id, std::string_view type,
      std::span<const std::string> methods, spt::ilp::APMRecord& apm, AuthFunction&& authfn ) -> Response
  {
    LOG_INFO << "Handling GET request for " << req.path << ". APM id: " << apm.id;

    auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    cp.values.try_emplace( model::ilp::name::APM_NOTE_VALUE, std::format( "List entity {} by reference {}", M::EntityType(), type ) );
    DEFER( spt::ilp::setDuration( cp ) );

    try
    {
      auto idx = apm.processes.size();
      auto [qstatus, eq] = parseQuery( req, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( qstatus != 200 || !eq )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid query" );
        return error( 400, "Invalid query"sv, methods, req.header, apm );
      }

      idx = apm.processes.size();
      auto [astatus, jwt] = authorise( req, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( astatus != 200 || !jwt )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid bearer token" );
        return error( astatus, methods, req.header, apm );
      }

      if ( !authfn( M::EntityType(), *jwt ) )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "User role not authorised" );
        return error( 401, methods, req.header, apm );
      }

      if ( auto t = magic_enum::enum_cast<model::EntityType>( type ); !t )
      {
        LOG_WARN << "User " << jwt->user.username << " specified invalid type " << type << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid type" );
        return error( 400, "Invalid type"sv, methods, req.header, apm );
      }

      auto query = bsoncxx::builder::stream::document{};
      query <<
        "references._id" << id <<
        "references.type" << type;
      if ( jwt->user.role != model::Role::superuser )
      {
        query << "customer.code" << jwt->user.customerCode;
      }

      if ( eq->after )
      {
        auto oid = spt::util::parseId( *eq->after );
        if ( !oid )
        {
          LOG_INFO << "Rejecting request for " << req.path << " with invalid after " << *eq->after << ". APM id: " << apm.id;
          cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid after parameter" );
          return error( 400, methods, req.header, apm );
        }

        query <<
          "_id" <<
            bsoncxx::builder::stream::open_document <<
              ( eq->descending ? "$lt" : "$gt" ) << *oid <<
            bsoncxx::builder::stream::close_document;
      }

      idx = apm.processes.size();
      auto [mstatus, m] = db::query<M>( query << bsoncxx::builder::stream::finalize, *eq, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( mstatus != 200 && mstatus != 404 )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Error retrieving entity" );
        return error( mstatus, "Error retrieving entity"sv, methods, req.header, apm );
      }

      if ( !m )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Database error" );
        return error( 404, methods, req.header, apm );
      }

      LOG_INFO << "Writing response for " << req.path << ". APM id: " << apm.id;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      idx = apm.processes.size();
      output( req, resp, *m, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      resp.correlationId = correlationId( req );
      resp.set( methods, Response::origins() );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, b, "BSON exception" );
      return error( 417, "Error creating entity"sv, methods, req.header, apm );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return error( 500, "Internal server error"sv, methods, req.header, apm );
  }

  template <Model M, typename AuthFunction>
  auto between( const spt::http2::framework::Request& req, std::string_view property,
      std::chrono::time_point<std::chrono::system_clock> start,
      std::chrono::time_point<std::chrono::system_clock> end,
      std::span<const std::string> methods, spt::ilp::APMRecord& apm, AuthFunction&& authfn ) -> Response
  {
    LOG_INFO << "Handling GET request for " << req.path << ". APM id: " << apm.id;

    auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    cp.values.try_emplace( model::ilp::name::APM_NOTE_VALUE, std::format( "List entity {} by date range", M::EntityType() ) );
    DEFER( spt::ilp::setDuration( cp ) );

    try
    {
      auto idx = apm.processes.size();
      auto [qstatus, eq] = parseQuery( req, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( qstatus != 200 || !eq )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid query" );
        return error( 400, "Invalid query"sv, methods, req.header, apm );
      }

      idx = apm.processes.size();
      auto [astatus, jwt] = authorise( req, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( astatus != 200 || !jwt )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid bearer token" );
        return error( astatus, methods, req.header, apm );
      }

      if ( !authfn( M::EntityType(), *jwt ) )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "User role not authorised" );
        return error( 401, methods, req.header, apm );
      }

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
          LOG_INFO << "Rejecting request for " << req.path << " with invalid after " << *eq->after << ". APM id: " << apm.id;
          cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid after parameter" );
          return error( 400, a.error(), methods, req.header, apm );
        }

        filter.after = a.value();
      }

      if ( jwt->user.role != model::Role::superuser ) filter.customer = jwt->user.customerCode;

      idx = apm.processes.size();
      auto [mstatus, m] = db::between<M>( std::move( filter ), *eq, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( mstatus != 200 && mstatus != 404 )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Error retrieving entity" );
        return error( mstatus, "Error retrieving entities"sv, methods, req.header, apm );
      }

      if ( !m )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Database error" );
        return error( 404, methods, req.header, apm );
      }

      LOG_INFO << "Writing response for " << req.path << ". APM id: " << apm.id;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      idx = apm.processes.size();
      output( req, resp, *m, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      resp.correlationId = correlationId( req );
      resp.set( methods, Response::origins() );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, b, "BSON exception" );
      return error( 417, "Error creating entity"sv, methods, req.header, apm );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return error( 500, "Internal server error"sv, methods, req.header, apm );
  }

  template <Model M, typename AuthFunction>
  auto between( const spt::http2::framework::RoutingRequest& req,
    boost::container::flat_map<std::string_view, std::string_view>&& args,
    std::span<const std::string> methods, spt::ilp::APMRecord& apm, AuthFunction&& authfn )
  {
    auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    DEFER( spt::ilp::setDuration( cp ) );

    auto type = args["type"sv];
    if ( type != "created"sv && type != "modified"sv )
    {
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid type" );
      return error( 404, methods, req.req.header, apm );
    }
    auto prop = std::format( "metadata.{}", type );

    const auto svar = spt::util::parseISO8601( args["start"sv] );
    if ( !svar.has_value() )
    {
      LOG_WARN << "Invalid start date in path " << req.req.path << ". APM id: " << apm.id;
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid start date" );
      return error( 400, svar.error(), methods, req.req.header, apm );
    }

    const auto evar = spt::util::parseISO8601( args["end"sv] );
    if ( !evar.has_value() )
    {
      LOG_WARN << "Invalid end date in path " << req.req.path << ". APM id: " << apm.id;
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid end date" );
      return error( 400, evar.error(), methods, req.req.header, apm );
    }

    auto idx = apm.processes.size();
    auto resp = between<M>( req.req, prop, svar.value(), evar.value(), methods, apm, std::forward<AuthFunction>( authfn ) );
    spt::ilp::addCurrentFunction( apm.processes[idx] );
    return resp;
  }

  template <Model M, typename AuthFunction>
  auto refcounts( const spt::http2::framework::Request& req, std::string_view entityId,
      std::span<const std::string> methods, spt::ilp::APMRecord& apm, AuthFunction&& authfn ) -> Response
  {
    LOG_INFO << "Handling GET request for " << req.path << ". APM id: " << apm.id;

    auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    cp.values.try_emplace( model::ilp::name::APM_NOTE_VALUE, std::format( "Count references to entity {}", M::EntityType() ) );
    cp.values.try_emplace( "entity_id", std::string{ entityId } );
    DEFER( spt::ilp::setDuration( cp ) );

    try
    {
      auto idx = apm.processes.size();
      auto [astatus, jwt] = authorise( req, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( astatus != 200 || !jwt )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid bearer token" );
        return error( astatus, methods, req.header, apm );
      }

      if ( !authfn( M::EntityType(), *jwt ) )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "User role not authorised" );
        return error( 401, methods, req.header, apm );
      }

      auto id = spt::util::parseId( entityId );
      if ( !id )
      {
        LOG_INFO << "Rejecting request for " << req.path << " with invalid id " << entityId << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid id" );
        return error( 400, methods, req.header, apm );
      }

      idx = apm.processes.size();
      auto [mstatus, m] = db::refcounts<M>( *id, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( mstatus != 200 && mstatus != 404 )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Error retrieving entity" );
        return error( mstatus, "Error counting references to entity"sv, methods, req.header, apm );
      }

      if ( mstatus == 404 || !m )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Database error" );
        return error( 404, methods, req.header, apm );
      }

      LOG_INFO << "Writing response for " << req.path << ". APM id: " << apm.id;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      idx = apm.processes.size();
      output( req, resp, *m, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      resp.correlationId = correlationId( req );
      resp.set( methods, Response::origins() );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, b, "BSON exception" );
      return error( 417, "Error creating entity"sv, methods, req.header, apm );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return error( 500, "Internal server error"sv, methods, req.header, apm );
  }

  template <Model M, typename AuthFunction>
  auto remove( const spt::http2::framework::Request& req, std::string_view entityId,
      std::span<const std::string> methods, spt::ilp::APMRecord& apm, AuthFunction&& authfn ) -> Response
  {
    LOG_INFO << "Handling DELETE request for " << req.path << ". APM id: " << apm.id;

    auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    cp.values.try_emplace( model::ilp::name::APM_NOTE_VALUE, std::format( "Remove entity {}", M::EntityType() ) );
    cp.values.try_emplace( "entity_id", std::string{ entityId } );
    DEFER( spt::ilp::setDuration( cp ) );

    try
    {
      auto idx = apm.processes.size();
      auto [astatus, jwt] = authorise( req, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( astatus != 200 || !jwt )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid bearer token" );
        return error( astatus, methods, req.header, apm );
      }

      if ( !authfn( M::EntityType(), *jwt ) )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "User role not authorised" );
        return error( 401, methods, req.header, apm );
      }

      auto id = spt::util::parseId( entityId );
      if ( !id )
      {
        LOG_INFO << "Rejecting request for " << req.path << " with invalid id " << entityId << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid id" );
        return error( 400, methods, req.header, apm );
      }

      idx = apm.processes.size();
      const auto mstatus = db::remove<M>( *id,
          jwt->user.role == model::Role::superuser ? ""sv : jwt->user.customerCode, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( mstatus != 200 && mstatus != 404 )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Error deleting entity" );
        return error( mstatus, "Error deleting entity"sv, methods, req.header, apm );
      }

      if ( mstatus == 404 )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Entity does not exist" );
        return error( mstatus, methods, req.header, apm );
      }

      LOG_INFO << "Writing response for " << req.path << ". APM id: " << apm.id;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      resp.correlationId = correlationId( req );
      resp.set( methods, Response::origins() );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, b, "BSON exception" );
      return error( 417, "Error creating entity"sv, methods, req.header, apm );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return error( 500, "Internal server error"sv, methods, req.header, apm );
  }

  template <Model M, typename AuthFunction>
  auto versionHistorySummary( const spt::http2::framework::Request& req, std::string_view entityId,
      std::span<const std::string> methods, spt::ilp::APMRecord& apm, AuthFunction&& authfn ) -> Response
  {
    LOG_INFO << "Handling GET request for " << req.path << ". APM id: " << apm.id;

    auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    cp.values.try_emplace( model::ilp::name::APM_NOTE_VALUE, std::format( "Retrieve version history summary for entity {}", M::EntityType() ) );
    cp.values.try_emplace( "entity_id", std::string{ entityId } );
    DEFER( spt::ilp::setDuration( cp ) );

    try
    {
      auto idx = apm.processes.size();
      auto [astatus, jwt] = authorise( req, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( astatus != 200 || !jwt )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid bearer token" );
        return error( astatus, methods, req.header, apm );
      }

      if ( !authfn( M::EntityType(), *jwt ) )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "User role not authorised" );
        return error( 401, methods, req.header, apm );
      }

      auto id = spt::util::parseId( entityId );
      if ( !id )
      {
        LOG_INFO << "Rejecting request for " << req.path << " with invalid id " << entityId << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid id" );
        return error( 400, methods, req.header, apm );
      }

      idx = apm.processes.size();
      auto [mstatus, m] = db::versionHistorySummary<M>( *id, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( mstatus != 200 && mstatus != 404 )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Error retrieving entity" );
        return error( mstatus, "Error retrieving entity history summaries"sv, methods, req.header, apm );
      }

      if ( !m )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Database error" );
        return error( 404, methods, req.header, apm );
      }

      LOG_INFO << "Writing response for " << req.path << ". APM id: " << apm.id;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      idx = apm.processes.size();
      output( req, resp, *m, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      resp.correlationId = correlationId( req );
      resp.set( methods, Response::origins() );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, b, "BSON exception" );
      return error( 417, "Error creating entity"sv, methods, req.header, apm );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return error( 500, "Internal server error"sv, methods, req.header, apm );
  }

  template <Model M, typename AuthFunction>
  auto versionHistoryDocument( const spt::http2::framework::Request& req, std::string_view historyId,
    std::span<const std::string> methods, spt::ilp::APMRecord& apm, AuthFunction authfn ) -> Response
  {
    LOG_INFO << "Handling GET request for " << req.path << ". APM id: " << apm.id;

    auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    cp.values.try_emplace( model::ilp::name::APM_NOTE_VALUE, std::format( "Retrieve version history document for entity {}", M::EntityType() ) );
    cp.values.try_emplace( "entity_id", std::string{ historyId } );
    DEFER( spt::ilp::setDuration( cp ) );

    try
    {
      auto idx = apm.processes.size();
      auto [astatus, jwt] = authorise( req, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( astatus != 200 || !jwt )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid bearer token" );
        return error( astatus, methods, req.header, apm );
      }

      if ( !authfn( M::EntityType(), *jwt ) )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "User role not authorised" );
        return error( 401, methods, req.header, apm );
      }

      auto id = spt::util::parseId( historyId );
      if ( !id )
      {
        LOG_INFO << "Rejecting request for " << req.path << " with invalid id " << historyId << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid id" );
        return error( 400, methods, req.header, apm );
      }

      idx = apm.processes.size();
      auto [mstatus, m] = db::versionHistoryDocument<M>( *id, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      if ( mstatus != 200 && mstatus != 404 )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Error retrieving entity" );
        return error( mstatus, "Error retrieving entity history document"sv, methods, req.header, apm );
      }

      if ( !m )
      {
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Database error" );
        return error( 404, methods, req.header, apm );
      }

      LOG_INFO << "Writing response for " << req.path << ". APM id: " << apm.id;
      auto resp = Response{ req.header };
      resp.jwt = jwt;
      idx = apm.processes.size();
      output( req, resp, *m, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      resp.correlationId = correlationId( req );
      resp.set( methods, Response::origins() );
      resp.entity = M::EntityType();
      return resp;
    }
    catch ( const bsoncxx::exception& b )
    {
      LOG_WARN << "BSON error processing " << req.path << ". " << b.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, b, "BSON exception" );
      return error( 417, "Error creating entity"sv, methods, req.header, apm );
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error processing request " << req.path << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
    }
    catch ( ... )
    {
      LOG_WARN << "Unexpected error processing request " << req.path << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return error( 500, "Internal server error"sv, methods, req.header, apm );
  }

  inline std::optional<model::Paginate> paginate( const model::EntitiesQuery& eq, spt::ilp::APMRecord& apm )
  {
    auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    cp.values.try_emplace( model::ilp::name::APM_NOTE_VALUE, "Retrieve paginate" );
    DEFER( spt::ilp::setDuration( cp ) );

    auto oid = spt::util::parseId( *eq.after );
    if ( !oid )
    {
      LOG_INFO << "Invalid after " << *eq.after << ". APM id: " << apm.id;
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Invalid after parameter" );
      return std::nullopt;
    }

    auto idx = apm.processes.size();
    auto [status, opt] = db::retrieve<model::Paginate>( *oid, ""sv, apm );
    spt::ilp::addCurrentFunction( apm.processes[idx] );
    if ( !opt )
    {
      LOG_WARN << "No entities retrieved after " << *oid << ". APM id: " << apm.id;
      cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Error retrieving entity" );
      return std::nullopt;
    }

    return std::move( opt );
  }
}

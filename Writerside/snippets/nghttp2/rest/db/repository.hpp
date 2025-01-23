//
// Created by Rakesh on 14/02/2021.
//

#pragma once

#include "filter/between.hpp"
#include "filter/id.hpp"
#include "filter/property.hpp"
#include "metadata.hpp"
#include "model/entities.hpp"
#include "model/entitiesquery.hpp"
#include "model/json.hpp"
#include "model/model.hpp"
#include "model/refcount.hpp"
#include "model/versionhistory.hpp"
#include "util/config.hpp"
#include "util/stacktrace.hpp"

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/json.hpp>
#include <chrono>
#include <fmt/format.h>
#include <log/NanoLog.hpp>
#include <mongo-service/api/api.hpp>
#include <mongo-service/api/repository/repository.hpp>
#include <mongo-service/common/util/bson.hpp>
#include <range/v3/range/concepts.hpp>
#include <range/v3/algorithm/find_if.hpp>

namespace spt::http2::rest::db
{
  using StatusResponse = std::tuple<int16_t, int64_t>;
  using std::operator""sv;
  using std::operator""s;
  using model::Model;

  inline std::string cacheKey( std::string_view type, bsoncxx::oid id )
  {
    return fmt::format( "/cache/entity/{}/id/{}", type, id.to_string() );
  }

  template <Model M>
  std::tuple<int16_t, int32_t> count( bsoncxx::document::view query )
  {
    auto count = spt::mongoservice::api::model::request::Count{ bsoncxx::document::value{ query } };
    count.database = M::Database();
    count.collection = M::Collection();
    count.options.emplace();
    count.options->limit = 10'000;
    count.options->maxTime = std::chrono::milliseconds{ 500 };

    auto result = spt::mongoservice::api::repository::count( count );
    if ( !result.has_value() )
    {
      LOG_WARN << "Error counting " << M::Database() << ':' << M::Collection() << ". " <<
        magic_enum::enum_name( result.error().cause ) << ". " << result.error().message;
      return { 417, 0 };
    }

    return { 200, result.value().count };
  }

  template <Model M>
  std::tuple<int16_t, std::optional<bsoncxx::oid>> lastId( bsoncxx::document::view query,
    bsoncxx::document::value&& projection, bsoncxx::document::value&& sort )
  {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::finalize;

    auto retrieve = spt::mongoservice::api::model::request::Retrieve{ bsoncxx::document::value{ query } };
    retrieve.database = M::Database();
    retrieve.collection = M::Collection();
    retrieve.options.emplace();
    retrieve.options->projection.emplace( std::move( projection ) );
    retrieve.options->limit = 1;
    retrieve.options->sort.emplace( std::move( sort ) );

    const auto result = spt::mongoservice::api::repository::retrieve<spt::mongoservice::api::model::request::IdFilter>( retrieve );
    if ( !result.has_value() )
    {
      LOG_WARN << "Error retrieving last document id from " << M::Database() << ':' << M::Collection() << ". " <<
        magic_enum::enum_name( result.error().cause ) << ". " << result.error().message;
      return { 417, std::nullopt };
    }

    if ( result.value().results.empty() )
    {
      LOG_INFO << "No last document id from " << M::Database() << ':' << M::Collection() << ". " << bsoncxx::to_json( query );
      return { 412, std::nullopt };
    }

    return { 200, result.value().results.front().id };
  }

  template <Model M>
  StatusResponse create( const M& m, bool skipVersion = false )
  {
    const auto st = std::chrono::steady_clock::now();

    try
    {
      auto now = std::chrono::system_clock::now();

      auto md = Metadata{};
      md.year = std::format( "{:%Y}", now );
      md.month = std::format( "{:%m}", now );

      auto cr = spt::mongoservice::api::model::request::CreateWithReference<M, Metadata>{ m, md };
      cr.database = M::Database();
      cr.collection = M::Collection();
      cr.skipVersion = skipVersion;

      const auto res = spt::mongoservice::api::repository::create( cr );
      if ( !res.has_value() )
      {
        LOG_WARN << "Error creating model " << spt::util::json::str( m ) << ". " << magic_enum::enum_name( res.error().cause ) << ". " << res.error().message;

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { 417, delta.count() };
      }

      if ( !skipVersion )
      {
        auto bson = spt::util::bson( m );
        auto bv = bson.view().get_document().value;
        util::Configuration::instance().set( cacheKey( M::EntityType(), m.id ),
            std::string{ reinterpret_cast<const char*>( bv.data() ), bv.length() }, 600 );
      }

      const auto et = std::chrono::steady_clock::now();
      const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
      return { 200, delta.count() };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error writing model " << ex.what() << ". " << spt::util::json::str( m );
      LOG_WARN << util::stacktrace();
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error writing model. " << spt::util::json::str( m );
      LOG_WARN << util::stacktrace();
    }

    const auto et = std::chrono::steady_clock::now();
    const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
    return { 422, delta.count() };
  }

  template <Model M>
  StatusResponse update( const M& m, bool skipVersion = false,
      std::optional<bsoncxx::oid> restoredFrom = std::nullopt )
  {
    const auto st = std::chrono::steady_clock::now();

    try
    {
      auto now = std::chrono::system_clock::now();
      auto filter = filter::Id{};
      filter.id = m.id;

      auto md = Metadata{};
      md.year = std::format( "{:%Y}", now );
      md.month = std::format( "{:%m}", now );
      if ( restoredFrom ) md.restoredFrom = *restoredFrom;

      auto ur = spt::mongoservice::api::model::request::ReplaceWithReference<M, Metadata, filter::Id>{ filter, m, md };
      ur.database = M::Database();
      ur.collection = M::Collection();
      ur.skipVersion = skipVersion;

      const auto res = spt::mongoservice::api::repository::update( ur );
      if ( !res.has_value() )
      {
        LOG_WARN << "Error replacing model " << spt::util::json::str( m ) << ". " << magic_enum::enum_name( res.error().cause ) << ". " << res.error().message;

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { 417, delta.count() };
      }

      auto bson = spt::util::bson( m );
      auto bv = bson.view().get_document().value;
      util::Configuration::instance().set( cacheKey( M::EntityType(), m.id ),
          std::string{ reinterpret_cast<const char*>( bv.data() ), bv.length() }, 600 );

      const auto et = std::chrono::steady_clock::now();
      const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
      return { 200, delta.count() };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error replacing model " << ex.what() << ". " << spt::util::json::str( m );
      LOG_WARN << util::stacktrace();
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error replacing model. " << spt::util::json::str( m );
      LOG_WARN << util::stacktrace();
    }

    const auto et = std::chrono::steady_clock::now();
    const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
    return { 422, delta.count() };
  }

  template <Model M>
  std::tuple<int16_t, int64_t, std::optional<M>> pipeline( std::vector<spt::mongoservice::api::model::request::Pipeline::Document::Stage> stages,
    bool caseInsensitive = false )
  {
    const auto st = std::chrono::steady_clock::now();

    try
    {
      auto model = spt::mongoservice::api::model::request::Pipeline{};
      model.database = M::Database();
      model.collection = M::Collection();
      if ( caseInsensitive )
      {
        model.options.emplace();
        model.options->collation.emplace();
        model.options->collation->locale = "en";
        model.options->collation->strength = 1;
        model.options->limit = 1;
      }

      model.document.specification.insert( model.document.specification.end(), std::make_move_iterator( stages.begin() ), std::make_move_iterator( stages.end() ) );
      stages.erase( stages.begin(), stages.end() );

      auto resp = spt::mongoservice::api::repository::pipeline<M>( model );
      if ( !resp.has_value() )
      {
        LOG_WARN << "Error executing query against " << M::Database() << ':' << M::Collection() << ". " <<
          spt::util::json::str( model.document ) << ". " << magic_enum::enum_name( resp.error().cause ) << ". " << resp.error().message;

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { 417, delta.count(), std::nullopt };
      }

      if ( resp.value().results.empty() )
      {
        LOG_WARN << "No matching documents for query against " << M::Database() << ':' << M::Collection() << ". " << spt::util::json::str( model.document );

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { 404, delta.count(), std::nullopt };
      }

      const auto et = std::chrono::steady_clock::now();
      const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
      return { 200, delta.count(), std::move( resp.value().results.front() ) };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error executing query against " << db << ':' << M::Collection() << ". " << ex.what();
      LOG_WARN << util::stacktrace();
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error executing query against " << db << ':' << M::Collection();
      LOG_WARN << util::stacktrace();
    }

    const auto et = std::chrono::steady_clock::now();
    const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
    return { 422, delta.count(), std::nullopt };
  }

  template <Model M>
  std::tuple<int16_t, int64_t, std::optional<model::Entities<M>>> pipeline( bsoncxx::document::value match,
      std::vector<spt::mongoservice::api::model::request::Pipeline::Document::Stage> stages,
      model::EntitiesQuery options )
  {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::finalize;
    using spt::mongoservice::api::execute;
    using spt::mongoservice::api::Request;

    const auto st = std::chrono::steady_clock::now();

    try
    {
      auto req = spt::mongoservice::api::model::request::Pipeline{};
      req.database = M::Database();
      req.collection = M::Collection();
      req.document.specification = std::move( stages );

      auto resp = spt::mongoservice::api::repository::pipeline<M>( req );
      if ( !resp.has_value() )
      {
        LOG_WARN << "Error executing query against " << M::Database() << ':' << M::Collection() << ". " <<
          spt::util::json::str( req.document ) << ". " << magic_enum::enum_name( resp.error().cause ) << ". " << resp.error().message;

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { 417, delta.count(), std::nullopt };
      }

      model::Entities<M> ms;
      ms.entities = std::move( resp.value().results );
      ms.page = std::size( ms.entities );

      if ( !options.after && ms.page > 0 && ms.page < options.limit )
      {
        ms.total = ms.page;
      }
      else
      {
        if ( const auto [cstatus, csize] = count<M>( match ); cstatus == 200 ) ms.total = csize;
      }

      if ( ms.page > 0 && ms.total != ms.page )
      {
        auto [lstatus, lv] = lastId<M>( match, document{} << "_id" << 1 << finalize,
          document{} << "_id" << (options.descending ? 1 : -1) << finalize );
        if ( lstatus != 200 )
        {
          const auto et = std::chrono::steady_clock::now();
          const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
          return { lstatus, delta.count(), std::nullopt };
        }

        auto lid = ms.entities.back().id;
        if ( lid != *lv ) ms.next = lid.to_string();
      }

      const auto et = std::chrono::steady_clock::now();
      const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
      return { 200, delta.count(), std::move( ms ) };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error executing query against " << M::Database() << ':' << M::Collection() << ". " << ex.what();
      LOG_WARN << util::stacktrace();
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error executing query against " << M::Database() << ':' << M::Collection();
      LOG_WARN << util::stacktrace();
    }

    const auto et = std::chrono::steady_clock::now();
    const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
    return { 422, delta.count(), std::nullopt };
  }

  template <Model M>
  std::tuple<int16_t, int64_t, std::optional<M>> retrieve( bsoncxx::oid id )
  {
    if ( auto p = model::pipeline<M>(); !p.empty() )
    {
      LOG_DEBUG << "Entity " << M::EntityType() << " requires pipeline";

      auto filter = filter::Id{ id };
      auto stages = std::vector<spt::mongoservice::api::model::request::Pipeline::Document::Stage>{};
      stages.reserve( p.size() + 1 );

      stages.emplace_back( "$match", spt::util::bson( filter ) );
      stages.insert( stages.end(), std::make_move_iterator( p.begin() ), std::make_move_iterator( p.end() ) );
      p.erase( p.begin(), p.end() );

      return pipeline<M>( std::move( stages ) );
    }

    const auto st = std::chrono::steady_clock::now();
    const auto ckey = cacheKey( M::EntityType(), id );
    if ( auto cached = util::Configuration::instance().get( ckey ); cached )
    {
      auto mv = bsoncxx::document::view( reinterpret_cast<const uint8_t*>( cached->data() ), cached->size() );
      auto m = M{ mv };

      const auto et = std::chrono::steady_clock::now();
      const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
      return { 200, delta.count(), std::move( m ) };
    }

    try
    {
      auto r = spt::mongoservice::api::model::request::Retrieve<filter::Id>{};
      r.database = M::Database();
      r.collection = M::Collection();
      r.document.emplace( id );

      auto resp = spt::mongoservice::api::repository::retrieve<M>( r );
      if ( !resp.has_value() )
      {
        LOG_WARN << "Error retrieving document " << M::Database() << ':' << M::Collection() << ':' << id.to_string() <<
           ". " << magic_enum::enum_name( resp.error().cause ) << ". " << resp.error().message;

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { resp.error().message == "Not found" ? 404 : 417, delta.count(), std::nullopt };
      }

      if ( !resp.value().result )
      {
        LOG_WARN << "No document " << M::Database() << ':' << M::Collection() << ':' << id.to_string();
        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { 404, delta.count(), std::nullopt };
      }

      const auto et = std::chrono::steady_clock::now();
      const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
      auto bson = spt::util::marshall( *resp.value().result );
      auto dv = bson.view();
      auto str = std::string{ reinterpret_cast<const char *>( dv.data() ), dv.length() };
      util::Configuration::instance().set( ckey, str, 600 );

      return { 200, delta.count(), std::move( resp.value().result ) };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error retrieving document " << M::Database() << ':' << M::Collection() << ':' << id.to_string() << ". " << ex.what();
      LOG_WARN << util::stacktrace();
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error retrieving document " << M::Database() << ':' << M::Collection() << ':' << id.to_string();
      LOG_WARN << util::stacktrace();
    }

    const auto et = std::chrono::steady_clock::now();
    const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
    return { 422, delta.count(), std::nullopt };
  }

  template<Model M, typename ValueType>
  std::tuple<int16_t, int64_t, std::optional<M>> retrieve( std::string_view property, ValueType value, bool caseInsensitive = false )
  {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::finalize;

    if ( auto p = model::pipeline<M>(); !p.empty() )
    {
      LOG_DEBUG << "Entity " << M::EntityType() << " requires pipeline";
      auto stages = std::vector<spt::mongoservice::api::model::request::Pipeline::Document::Stage>{};
      stages.reserve( 3 + p.size() );

      auto match = filter::Property<ValueType>{ property, value, customer };
      stages.emplace_back( "$match", spt::util::bson( match ) );
      stages.emplace_back( "$sort", document{} << "_id" << -1 << finalize );
      stages.emplace_back( "$limit", bsoncxx::types::b_int32{ 1 } );

      stages.insert( stages.end(), std::make_move_iterator( p.begin() ), std::make_move_iterator( p.end() ) );
      p.erase( p.begin(), p.end() );

      return pipeline<M>( std::move( stages ), database, caseInsensitive );
    }

    const auto st = std::chrono::steady_clock::now();

    try
    {
      auto r = spt::mongoservice::api::model::request::Retrieve<filter::Property<ValueType>>{};
      r.database = M::Database();
      r.collection = M::Collection();
      r.document.emplace( property, value, customer );
      r.options.emplace();
      r.options->collation.emplace();
      r.options->collation->locale = "en";
      r.options->collation->strength = 1;
      r.options->limit = 1;
      r.options->sort = document{} << "_id" << -1 << finalize;

      auto resp = spt::mongoservice::api::repository::retrieve<M>( r );
      if ( !resp.has_value() )
      {
        LOG_WARN << "Unable to retrieve document from " << M::Database() << ':' << M::Collection() <<
          " with property: " << property << ", value: " << value <<
          ". " << magic_enum::enum_name( resp.error().cause ) << ". " << resp.error().message;

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { resp.error().message == "Not found" ? 404 : 417, delta.count(), std::nullopt };
      }

      if ( resp.value().results.empty() )
      {
        LOG_WARN << "No matching document " << M::Database() << ':' << M::Collection() <<
          " with property: " << property << ", value: " << value;

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { 404, delta.count(), std::nullopt };
      }

      const auto et = std::chrono::steady_clock::now();
      const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
      return { 200, delta.count(), std::move( resp.value().results.front() ) };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error retrieving document " << M::Database() << ':' << M::Collection() <<
          " with property: " << property << ", value: " << value <<
          ". " << ex.what();
      LOG_WARN << util::stacktrace();
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error retrieving document " << M::Database() << ':' << M::Collection() <<
          " with property: " << property << ", value: " << value;
      LOG_WARN << util::stacktrace();
    }

    const auto et = std::chrono::steady_clock::now();
    const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
    return { 422, delta.count(), std::nullopt };
  }

  template <Model M>
  std::tuple<int16_t, int64_t, std::optional<model::Entities<M>>> query( bsoncxx::document::value query, model::EntitiesQuery options )
  {
    using bsoncxx::builder::stream::array;
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::finalize;

    if ( auto p = model::pipeline<M>(); !p.empty() )
    {
      LOG_DEBUG << "Entity " << M::EntityType() << " requires pipeline";

      auto stages = std::vector<spt::mongoservice::api::model::request::Pipeline::Document::Stage>{};
      stages.reserve( p.size() + 3 );
      stages.emplace_back( "$match", query.view() );
      stages.emplace_back( "$sort", document{} << "_id" << (options.descending ? -1 : 1) << finalize );
      stages.emplace_back( "$limit", bsoncxx::types::b_int32{ 1 } );

      stages.insert( stages.end(), std::make_move_iterator( p.begin() ), std::make_move_iterator( p.end() ) );
      p.erase( p.begin(), p.end() );

      return pipeline<M>( std::move( query ), std::move( stages ), options, database );
    }

    const auto st = std::chrono::steady_clock::now();

    try
    {
      auto retrieve = spt::mongoservice::api::model::request::Retrieve{ bsoncxx::document::value{ query } };
      retrieve.database = M::Database();
      retrieve.collection = M::Collection();
      retrieve.options.emplace();
      retrieve.options->limit = options.limit;
      retrieve.options->sort = document{} << "_id" << (options.descending ? -1 : 1) << finalize;

      auto result = spt::mongoservice::api::repository::retrieve<M>( retrieve );
      if ( !result.has_value() )
      {
        LOG_WARN << "Unable to execute query against " << M::Database() << ':' << M::Collection() << ". " <<
          magic_enum::enum_name( result.error().cause ) << ". " << result.error().message;

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { 417, delta.count(), std::nullopt };
      }

      model::Entities<M> ms;
      ms.entities = std::move( result.value().results );
      ms.page = std::size( ms.entities );

      if ( !options.after && ms.page > 0 && ms.page < options.limit )
      {
        ms.total = ms.page;
      }
      else
      {
        if ( const auto [cstatus, csize] = count<M>( query ); cstatus == 200 ) ms.total = csize;
      }

      if ( ms.page > 0 && ms.total != ms.page )
      {
        auto [lstatus, lv] = lastId<M>( query, document{} << "_id" << 1 << finalize,
          document{} << "_id" << (options.descending ? 1 : -1) << finalize );
        if ( lstatus != 200 )
        {
          const auto et = std::chrono::steady_clock::now();
          const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
          return { lstatus, delta.count(), std::nullopt };
        }

        auto lid = ms.entities.back().id;
        if ( lid != *lv ) ms.next = lid.to_string();
      }

      const auto et = std::chrono::steady_clock::now();
      const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
      return { 200, delta.count(), std::move( ms ) };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error executing query against " << M::Database() << ':' << M::Collection() << ". " << ex.what();
      LOG_WARN << util::stacktrace();
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error executing query against " << M::Database() << ':' << M::Collection();
      LOG_WARN << util::stacktrace();
    }

    const auto et = std::chrono::steady_clock::now();
    const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
    return { 422, delta.count(), std::nullopt };
  }

  template <Model M>
  std::tuple<int16_t, int64_t, std::optional<std::vector<M>>> rawquery(
      bsoncxx::document::value query, model::EntitiesQuery options )
  {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::finalize;

    const auto st = std::chrono::steady_clock::now();
    const auto db = database.empty() ? M::Database() : database;

    try
    {
      auto retrieve = spt::mongoservice::api::model::request::Retrieve{ bsoncxx::document::value{ query } };
      retrieve.database = M::Database();
      retrieve.collection = M::Collection();
      retrieve.options.emplace();
      retrieve.options->limit = options.limit;
      retrieve.options->sort = document{} << "_id" << (options.descending ? -1 : 1) << finalize;

      auto result = spt::mongoservice::api::repository::retrieve<M>( retrieve );
      if ( !result.has_value() )
      {
        LOG_WARN << "Unable to execute query against " << M::Database() << ':' << M::Collection() << ". " <<
          magic_enum::enum_name( result.error().cause ) << ". " << result.error().message;

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { 417, delta.count(), std::nullopt };
      }

      const auto et = std::chrono::steady_clock::now();
      const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
      return { 200, delta.count(), std::move( result.value().results ) };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error executing query against " << M::Database() << ':' << M::Collection() << ". " << ex.what();
      LOG_WARN << util::stacktrace();
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error executing query against " << M::Database() << ':' << M::Collection();
      LOG_WARN << util::stacktrace();
    }

    const auto et = std::chrono::steady_clock::now();
    const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
    return { 422, delta.count(), std::nullopt };
  }

  template <Model M>
  std::tuple<int16_t, int64_t> transaction( bsoncxx::array::view items )
  {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::finalize;
    using spt::mongoservice::api::execute;
    using spt::mongoservice::api::Request;

    const auto st = std::chrono::steady_clock::now();

    try
    {
      auto req = Request::transaction( std::string{ M::Database() }, std::string{ M::Collection() },
          document{} << "items" << items << finalize );
      const auto [type, opt] = execute( req );
      if ( !opt )
      {
        LOG_WARN << "Unable to execute transaction against " << db << ':' << M::Collection() << ". " << req.document;

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { 424, delta.count() };
      }

      const auto view = opt->view();
      if ( const auto err = spt::util::bsonValueIfExists<std::string>( "error", view ); err )
      {
        LOG_WARN << "Error executing query against " << db << ':' << M::Collection() << ". " << req.document << ". " << view;

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { 417, delta.count() };
      }

      LOG_INFO << "Transaction results " << view;
      const auto et = std::chrono::steady_clock::now();
      const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
      return { 200, delta.count() };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error executing query against " << M::Database() << ':' << M::Collection() << ". " << ex.what();
      LOG_WARN << util::stacktrace();
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error executing query against " << M::Database() << ':' << M::Collection();
      LOG_WARN << util::stacktrace();
    }

    const auto et = std::chrono::steady_clock::now();
    const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
    return { 422, delta.count() };
  }

  template <Model M>
  std::tuple<int16_t, int64_t, std::optional<std::vector<M>>> rawpipeline( std::vector<spt::mongoservice::api::model::request::Pipeline::Document::Stage> stages )
  {
    const auto st = std::chrono::steady_clock::now();

    try
    {
      auto req = spt::mongoservice::api::model::request::Pipeline{};
      req.database = M::Database();
      req.collection = M::Collection();
      req.document.specification = std::move( stages );

      auto res = spt::mongoservice::api::repository::pipeline<M>( req );
      if ( !res.has_value() )
      {
        LOG_WARN << "Error executing query against " << M::Database() << ':' << M::Collection() << ". " <<
          spt::util::json::str( req.document ) << ". " << magic_enum::enum_name( res.error().cause ) << ". " << res.error().message;

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { 417, delta.count(), std::nullopt };
      }

      const auto et = std::chrono::steady_clock::now();
      const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
      return { 200, delta.count(), std::move( res.value().results ) };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error executing query against " << M::Database() << ':' << M::Collection() << ". " << ex.what();
      LOG_WARN << util::stacktrace();
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error executing query against " << M::Database() << ':' << M::Collection();
      LOG_WARN << util::stacktrace();
    }

    const auto et = std::chrono::steady_clock::now();
    const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
    return { 422, delta.count(), std::nullopt };
  }

  template <Model M>
  std::tuple<int16_t, int64_t, std::optional<model::Entities<M>>> between( const filter::Between& filter, model::EntitiesQuery options )
  {
    using bsoncxx::builder::stream::array;
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::finalize;
    using spt::mongoservice::api::execute;
    using spt::mongoservice::api::Request;

    if ( auto p = model::pipeline<M>(); !p.empty() )
    {
      LOG_DEBUG << "Entity " << M::EntityType() << " requires pipeline";

      auto stages = std::vector<spt::mongoservice::api::model::request::Pipeline::Document::Stage>{};
      stages.reserve( p.size() + 3 );
      stages.emplace_back( "$match", spt::util::bson( filter ) );
      stages.emplace_back( "$sort", document{} << filter.field << (options.descending ? -1 : 1) << finalize );
      stages.emplace_back( "$limit", bsoncxx::types::b_int32{ options.limit } );

      stages.insert( stages.end(), std::make_move_iterator( p.begin() ), std::make_move_iterator( p.end() ) );
      p.erase( p.begin(), p.end() );

      return pipeline<M>( spt::util::marshall( filter ), std::move( stages ), options, database );
    }

    const auto st = std::chrono::steady_clock::now();

    try
    {
      auto retrieve = spt::mongoservice::api::model::request::Retrieve{ spt::util::marshall( filter ) };
      retrieve.database = M::Database();
      retrieve.collection = M::Collection();
      retrieve.options.emplace();
      retrieve.options->limit = options.limit;
      retrieve.options->sort = document{} << filter.field << (options.descending ? -1 : 1) << finalize;
      auto res = spt::mongoservice::api::repository::retrieve<M>( retrieve );

      if ( !res.has_value() )
      {
        LOG_WARN << "Error executing query against " << M::Database() << ':' << M::Collection() << ". " <<
          magic_enum::enum_name( res.error().cause ) << ". " << res.error().message;

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { 417, delta.count(), std::nullopt };
      }

      model::Entities<M> ms;
      ms.entities = std::move( res.value().results );
      ms.page = std::size( ms.entities );

      if ( !options.after && ms.page > 0 && ms.page < options.limit )
      {
        ms.total = ms.page;
      }
      else
      {
        if ( const auto [cstatus, csize] = count<M>( *retrieve.document ); cstatus == 200 ) ms.total = csize;
      }

      if ( ms.page > 0 && ms.total != ms.page )
      {
        auto [lstatus, lv] = lastId<M>( *retrieve.document,
          document{} << filter.field << 1 << finalize,
          document{} << filter.field << (options.descending ? 1 : -1) << finalize );
        if ( lstatus != 200 )
        {
          const auto et = std::chrono::steady_clock::now();
          const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
          return { lstatus, delta.count(), std::nullopt };
        }

        if ( ms.entities.back().id != *lv )
        {
          ms.next = spt::util::isoDateMillis( filter.field == "created"sv ? ms.entities.back().metadata.created : ms.entities.back().metadata.modified );
        }
      }

      const auto et = std::chrono::steady_clock::now();
      const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
      return { 200, delta.count(), std::move( ms ) };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error executing query against " << M::Database() << ':' << M::Collection() << ". " << ex.what();
      LOG_WARN << util::stacktrace();
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error executing query against " << M::Database() << ':' << M::Collection();
      LOG_WARN << util::stacktrace();
    }

    const auto et = std::chrono::steady_clock::now();
    const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
    return { 422, delta.count(), std::nullopt };
  }

  template <Model M>
  std::tuple<int16_t, int64_t, std::optional<model::RefCounts>> refcounts( bsoncxx::oid id )
  {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::finalize;
    using spt::mongoservice::api::execute;
    using spt::mongoservice::api::Request;

    const auto refs = M::References();
    if ( refs.empty() ) return { 200, 0, std::nullopt };

    const auto st = std::chrono::steady_clock::now();

    try
    {
      model::RefCounts rf;
      rf.references.reserve( refs.size() );
      int64_t time{ 0 };

      for ( const auto& ref : refs )
      {
        auto req = spt::mongoservice::api::model::request::Count<bsoncxx::document::value>{ document{} << ref.field << id << finalize };
        req.database = model::database( ref.type );
        req.collection = model::collection( ref.type );

        const auto resp = spt::mongoservice::api::repository::count( req );
        if ( !resp.has_value() )
        {
          LOG_WARN << "Error refcounting document " << M::Database() << ':' << M::Collection() << ':' << id.to_string() <<
            " in collection " << model::collection( ref.type ) << ". " <<
              magic_enum::enum_name( resp.error().cause ) << ". " << resp.error().message;

          const auto et = std::chrono::steady_clock::now();
          const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
          time += delta.count();
          return { 417, time, std::nullopt };
        }

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        time += delta.count();
        rf.references.emplace_back( model::RefCount{ static_cast<int32_t>( resp.value().count ), ref.type } );
      }

      return { 200, time, std::move( rf ) };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error retrieving reference counts for " << M::Database() << ':' << M::Collection() << ':' << id.to_string() <<
         ". " << ex.what();
      LOG_WARN << util::stacktrace();
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error retrieving document " << M::Database() << ':' << M::Collection() << ':' << id.to_string();
      LOG_WARN << util::stacktrace();
    }

    const auto et = std::chrono::steady_clock::now();
    const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
    return { 422, delta.count(), std::nullopt };
  }

  template <Model M>
  std::tuple<int16_t, int64_t> remove( bsoncxx::oid id )
  {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::finalize;
    using spt::mongoservice::api::execute;
    using spt::mongoservice::api::Request;

    const auto st = std::chrono::steady_clock::now();

    try
    {
      auto [rstatus, rtime, rc] = refcounts<M>( id );
      if ( rstatus != 200 ) return { rstatus, rtime };
      if ( rc )
      {
        for ( const auto& r : rc->references )
        {
          if ( r.count > 0 )
          {
            LOG_INFO << "Rejecting delete document " << M::Database() << ':' <<
              M::Collection() << ':' << id.to_string() <<
              " as it is being referenced. " << spt::util::json::str( *rc );
            return { 412, rtime };
          }
        }
      }

      auto now = std::chrono::system_clock::now();
      auto req = spt::mongoservice::api::model::request::Delete<filter::Id, Metadata>( filter::Id{} );
      req.database = M::Database();
      req.collection = M::Collection();
      req.document->id = id;
      if ( !customer.empty() ) req.document->customer = customer;
      req.metadata.emplace();
      req.metadata->year = std::format( "{:%Y}", now );
      req.metadata->month = std::format( "{:%m}", now );

      auto resp = spt::mongoservice::api::repository::remove( req );

      if ( !resp.has_value() )
      {
        LOG_WARN << "Error deleting document " << M::Database() << ':' << M::Collection() << ':' << id.to_string() << ". " <<
          magic_enum::enum_name( resp.error().cause ) << ". " << resp.error().message;

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { 417, delta.count() };
      }

      auto iter = ranges::find_if( resp.value().success, [&id]( const bsoncxx::oid& oid ) { return oid == id; } );
      if ( iter == ranges::end( resp.value().success ) )
      {
        LOG_WARN << "No document deleted for " << M::Database() << ':' << M::Collection() << ':' << id.to_string();

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { 417, delta.count() };
      }

      util::Configuration::instance().remove( cacheKey( M::EntityType(), id ) );
      const auto et = std::chrono::steady_clock::now();
      const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
      return { 200, delta.count() };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error removing document " << M::Database() << ':' << M::Collection() << ':' << id.to_string() <<
        ". " << ex.what();
      LOG_WARN << util::stacktrace();
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error removing document " << M::Database() << ':' << M::Collection() << ':' << id.to_string();
      LOG_WARN << util::stacktrace();
    }

    const auto et = std::chrono::steady_clock::now();
    const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
    return { 422, delta.count() };
  }

  template <Model M>
  std::tuple<int16_t, int64_t> remove( bsoncxx::document::value&& query )
  {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::finalize;
    using spt::mongoservice::api::execute;
    using spt::mongoservice::api::Request;

    const auto st = std::chrono::steady_clock::now();

    try
    {
      const auto now = std::chrono::system_clock::now();
      auto req = spt::mongoservice::api::model::request::Delete<bsoncxx::document::value, Metadata>{ std::move( query )  };
      req.database = M::Database();
      req.collection = M::Collection();
      req.metadata.emplace();
      req.metadata->year = std::format( "{:%Y}", now );
      req.metadata->month = std::format( "{:%m}", now );

      auto resp = spt::mongoservice::api::repository::remove( req );
      if ( !resp.has_value() )
      {
        LOG_WARN << "Error deleting document(s) in " << M::Database() << ':' << M::Collection() << ". " <<
          bsoncxx::to_json( req.document->view() ) << ". " <<
          magic_enum::enum_name( resp.error().cause ) << ". " << resp.error().message;

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { 417, delta.count() };
      }

      if ( resp.value().success.empty() )
      {
        LOG_WARN << "No documents deleted for " << M::Database() << ':' << M::Collection() << bsoncxx::to_json( req.document->view() );

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { 204, delta.count() };
      }

      const auto et = std::chrono::steady_clock::now();
      const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
      return { 200, delta.count() };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error removing document(s) in " << M::Database() << ':' << M::Collection() <<
        ". " << ex.what();
      LOG_WARN << util::stacktrace();
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error removing document(s) in " << M::Database() << ':' << M::Collection() <<
      LOG_WARN << util::stacktrace();
    }

    const auto et = std::chrono::steady_clock::now();
    const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
    return { 422, delta.count() };
  }

  template <Model M>
  std::tuple<int16_t, int64_t, std::optional<model::Entities<model::VersionHistorySummary>>> versionHistorySummary( bsoncxx::oid id )
  {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::finalize;
    using spt::mongoservice::api::execute;
    using spt::mongoservice::api::Request;

    const auto st = std::chrono::steady_clock::now();

    try
    {
      auto req = Request::retrieve( std::string{ model::VersionHistorySummary::Database() }, std::string{ model::VersionHistorySummary::Collection() },
          document{} <<
            "database" << M::Database() <<
            "collection" << M::Collection() <<
            "entity._id" << id <<
          finalize );
      req.options = document{} <<
        "projection" <<
          open_document <<
            "_id" << 1 <<
            "action" << 1 <<
            "created" << 1 <<
            "metadata" << 1 <<
          close_document <<
        "sort" << open_document << "_id" << 1 << close_document <<
        finalize;

      const auto [type, opt] = execute( req );
      if ( !opt )
      {
        LOG_WARN << "Unable to retrieve version history summary documents for " <<
          M::Database() << ':' << M::Collection() << ':' << id.to_string() << ". " << req.document << ". " << *req.options;

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { 424, delta.count(), std::nullopt };
      }

      const auto view = opt->view();
      if ( const auto err = spt::util::bsonValueIfExists<std::string>( "error", view ); err )
      {
        LOG_WARN << "Error retrieving version history summary documents for " <<
          M::Database() << ':' << M::Collection() << ':' << id <<
          ". " << req.document << ". " << *req.options << ". " << view;

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { 417, delta.count(), std::nullopt };
      }

      auto arr = spt::util::bsonValueIfExists<bsoncxx::array::view>( "results", view );
      if ( !arr )
      {
        LOG_WARN << "No version history summary documents for " <<
          M::Database() << ':' << M::Collection() << ':' << id.to_string() <<
          ". " << req.document << ". " << *req.options << ". " << view;

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { 404, delta.count(), std::nullopt };
      }

      model::Entities<model::VersionHistorySummary> ms;
      ms.entities.reserve( 8 );
      std::for_each( std::cbegin( *arr ), std::cend( *arr ),
          [&ms]( const auto& d ) { ms.entities.emplace_back( d.get_document().value ); } );
      ms.page = std::size( ms.entities );
      ms.total = ms.page;

      const auto et = std::chrono::steady_clock::now();
      const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
      return { 200, delta.count(), std::move( ms ) };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error retrieving version history summary documents for " <<
        M::Database() << ':' << M::Collection() << ':' << id.to_string() <<
        ". " << ex.what();
      LOG_WARN << util::stacktrace();
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error retrieving version history documents for " <<
        M::Database() << ':' << M::Collection() << ':' << id.to_string();
      LOG_WARN << util::stacktrace();
    }

    const auto et = std::chrono::steady_clock::now();
    const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
    return { 422, delta.count(), std::nullopt };
  }

  template <Model M>
  std::tuple<int16_t, int64_t, std::optional<model::VersionHistoryDocument<M>>> versionHistoryDocument( bsoncxx::oid id )
  {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::finalize;
    using spt::mongoservice::api::execute;
    using spt::mongoservice::api::Request;

    const auto st = std::chrono::steady_clock::now();

    try
    {
      auto req = Request::retrieve( std::string{ model::VersionHistorySummary::Database() }, std::string{ model::VersionHistorySummary::Collection() },
          document{} <<
            "_id" << id <<
            "database" << M::Database() <<
            "collection" << M::Collection() <<
          finalize );
      const auto [type, opt] = execute( req );
      if ( !opt )
      {
        LOG_WARN << "Unable to retrieve version history document for " <<
          M::Database() << ':' << M::Collection() << " with id: " << id << ". " << req.document;

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { 424, delta.count(), std::nullopt };
      }

      const auto view = opt->view();
      if ( const auto err = spt::util::bsonValueIfExists<std::string>( "error", view ); err )
      {
        LOG_WARN << "Error retrieving version history document for " <<
          M::Database() << ':' << M::Collection() << " with id: " << id << ". " << req.document << ". " << view;

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { 417, delta.count(), std::nullopt };
      }

      auto doc = spt::util::bsonValueIfExists<bsoncxx::document::view>( "result", view );
      if ( !doc )
      {
        LOG_WARN << "No version history document for " <<
          M::Database() << ':' << M::Collection() << " with id: " << id << ". " << req.document << ". " << view;

        const auto et = std::chrono::steady_clock::now();
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
        return { 404, delta.count(), std::nullopt };
      }

      model::VersionHistoryDocument<M> ms{ *doc };
      const auto et = std::chrono::steady_clock::now();
      const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
      return { 200, delta.count(), std::move( ms ) };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error retrieving version history document for " <<
        M::Database() << ':' << M::Collection() << " with id: " << id.to_string() <<
        ". " << ex.what();
      LOG_WARN << util::stacktrace();
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error retrieving version history document for " <<
        M::Database() << ':' << M::Collection() << " with id: " << id.to_string();
      LOG_WARN << util::stacktrace();
    }

    const auto et = std::chrono::steady_clock::now();
    const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>( et - st );
    return { 422, delta.count(), std::nullopt };
  }
}

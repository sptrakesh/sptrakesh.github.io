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
#include "model/ilp.hpp"
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
#include <ilp/apmrecord.hpp>
#include <log/NanoLog.hpp>
#include <mongo-service/api/api.hpp>
#include <mongo-service/api/repository/repository.hpp>
#include <mongo-service/common/util/bson.hpp>
#include <mongo-service/common/util/defer.hpp>
#include <range/v3/range/concepts.hpp>
#include <range/v3/algorithm/find_if.hpp>

namespace spt::db
{
  using std::operator""sv;
  using std::operator""s;
  using model::Model;

  inline std::string cacheKey( std::string_view type, bsoncxx::oid id )
  {
    return fmt::format( "/cache/entity/{}/id/{}", type, id.to_string() );
  }

  template <Model M>
  std::tuple<int16_t, int32_t> count( bsoncxx::document::view query, spt::ilp::APMRecord& apm )
  {
    auto& p = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    DEFER( spt::ilp::setDuration( p ) );

    auto count = spt::mongoservice::api::model::request::Count{ bsoncxx::document::value{ query } };
    count.database = M::Database();
    count.collection = M::Collection();
    count.options.emplace();
    count.options->limit = 10'000;
    count.options->maxTime = std::chrono::milliseconds{ 500 };
    count.correlationId = apm.id;

    auto result = spt::mongoservice::api::repository::count( count );
    if ( !result.has_value() )
    {
      LOG_WARN << "Error counting " << M::Database() << ':' << M::Collection() << ". " <<
        magic_enum::enum_name( result.error().cause ) << ". " << result.error().message << ". APM id: " << apm.id;
      p.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, std::format( "Database error. {}", result.error().message ) );
      return { 417, 0 };
    }

    return { 200, result.value().count };
  }

  template <Model M>
  std::tuple<int16_t, std::optional<bsoncxx::oid>> lastId( bsoncxx::document::view query,
    bsoncxx::document::value&& projection, bsoncxx::document::value&& sort, spt::ilp::APMRecord& apm )
  {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::finalize;

    auto& p = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    DEFER( spt::ilp::setDuration( p ) );

    auto retrieve = spt::mongoservice::api::model::request::Retrieve{ bsoncxx::document::value{ query } };
    retrieve.database = M::Database();
    retrieve.collection = M::Collection();
    retrieve.options.emplace();
    retrieve.options->projection.emplace( std::move( projection ) );
    retrieve.options->limit = 1;
    retrieve.options->sort.emplace( std::move( sort ) );
    retrieve.correlationId = apm.id;

    const auto result = spt::mongoservice::api::repository::retrieve<spt::mongoservice::api::model::request::IdFilter>( retrieve );
    if ( !result.has_value() )
    {
      LOG_WARN << "Error retrieving last document id from " << M::Database() << ':' << M::Collection() << ". " <<
        magic_enum::enum_name( result.error().cause ) << ". " << result.error().message << ". APM id: " << apm.id;
      p.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, std::format( "Database error. {}", result.error().message ) );
      return { 417, std::nullopt };
    }

    if ( result.value().results.empty() )
    {
      LOG_INFO << "No last document id from " << M::Database() << ':' << M::Collection() << ". " << bsoncxx::to_json( query ) << ". APM id: " << apm.id;
      p.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "No results returned." );
      return { 412, std::nullopt };
    }

    return { 200, result.value().results.front().id };
  }

  template <Model M>
  uint16_t create( const M& m, spt::ilp::APMRecord& apm, bool skipVersion = false )
  {
    auto& p = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    DEFER( spt::ilp::setDuration( p ) );

    try
    {
      auto now = std::chrono::system_clock::now();
      auto md = Metadata{};
      md.year = std::format( "{:%Y}", now );
      md.month = std::format( "{:%m}", now );

      auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Step );
      spt::ilp::addCurrentFunction( cp );
      auto cr = spt::mongoservice::api::model::request::CreateWithReference<M, Metadata>{ m, md };
      cr.database = M::Database();
      cr.collection = M::Collection();
      cr.skipVersion = skipVersion;
      cr.correlationId = apm.id;

      const auto res = spt::mongoservice::api::repository::create( cr );
      spt::ilp::setDuration( cp );
      if ( !res.has_value() )
      {
        LOG_WARN << "Error creating model " << spt::util::json::str( m ) << ". " << magic_enum::enum_name( res.error().cause ) << ". " << res.error().message << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, std::format( "Database error. {}", res.error().message ) );
        return 417;
      }

      if ( !skipVersion )
      {
        auto& step = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Step );
        DEFER( spt::ilp::setDuration( step ) );
        step.values.try_emplace( model::ilp::name::APM_NOTE_VALUE, "Cache model" );
        auto bson = spt::util::bson( m );
        auto bv = bson.view().get_document().value;
        util::Configuration::instance().set( cacheKey( M::EntityType(), m.id ),
            std::string{ reinterpret_cast<const char*>( bv.data() ), bv.length() }, 600 );
      }

      return 200;
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error writing model " << ex.what() << ". " << spt::util::json::str( m ) << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
      return 422;
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error writing model. " << spt::util::json::str( m ) << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      p.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return 500;
  }

  template <Model M>
  uint16_t update( const M& m, std::string_view customer, spt::ilp::APMRecord& apm,
      bool skipVersion = false, std::optional<bsoncxx::oid> restoredFrom = std::nullopt )
  {
    auto& p = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    DEFER( spt::ilp::setDuration( p ) );

    try
    {
      auto filter = filter::Id{};
      filter.id = m.id;
      filter.customer = customer;

      auto now = std::chrono::system_clock::now();
      auto md = Metadata{};
      md.year = std::format( "{:%Y}", now );
      md.month = std::format( "{:%m}", now );
      if ( restoredFrom ) md.restoredFrom = *restoredFrom;

      auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Step );
      spt::ilp::addCurrentFunction( cp );
      auto ur = spt::mongoservice::api::model::request::ReplaceWithReference<M, Metadata, filter::Id>{ filter, m, md };
      ur.database = M::Database();
      ur.collection = M::Collection();
      ur.skipVersion = skipVersion;
      ur.correlationId = apm.id;

      const auto res = spt::mongoservice::api::repository::update( ur );
      spt::ilp::setDuration( cp );
      if ( !res.has_value() )
      {
        LOG_WARN << "Error replacing model " << spt::util::json::str( m ) << ". " << magic_enum::enum_name( res.error().cause ) << ". " << res.error().message << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, std::format( "Database error. {}", res.error().message ) );
        return 417;
      }

      auto& step = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Step );
      DEFER( spt::ilp::setDuration( step ) );
      step.values.try_emplace( model::ilp::name::APM_NOTE_VALUE, "Cache model" );

      auto bson = spt::util::bson( m );
      auto bv = bson.view().get_document().value;
      util::Configuration::instance().set( cacheKey( M::EntityType(), m.id ),
          std::string{ reinterpret_cast<const char*>( bv.data() ), bv.length() }, 600 );

      return 200;
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error replacing model " << ex.what() << ". " << spt::util::json::str( m ) << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
      return 422;
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error replacing model. " << spt::util::json::str( m ) << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      p.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return 500;
  }

  template <Model M>
  std::tuple<int16_t, std::optional<M>> pipeline( std::vector<spt::mongoservice::api::model::request::Pipeline::Document::Stage> stages,
    spt::ilp::APMRecord& apm, bool caseInsensitive = false )
  {
    auto& p = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    DEFER( spt::ilp::setDuration( p ) );

    try
    {
      auto model = spt::mongoservice::api::model::request::Pipeline{};
      model.database = M::Database();
      model.collection = M::Collection();
      model.options.emplace();
      model.options->limit = 1;
      model.correlationId = apm.id;
      if ( caseInsensitive )
      {
        model.options->collation.emplace();
        model.options->collation->locale = "en";
        model.options->collation->strength = 1;
      }

      model.document.specification.insert( model.document.specification.end(), std::make_move_iterator( stages.begin() ), std::make_move_iterator( stages.end() ) );
      stages.erase( stages.begin(), stages.end() );

      auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Step );
      spt::ilp::addCurrentFunction( cp );
      auto resp = spt::mongoservice::api::repository::pipeline<M>( model );
      spt::ilp::setDuration( cp );

      if ( !resp.has_value() )
      {
        LOG_WARN << "Error executing query against " << M::Database() << ':' << M::Collection() << ". " <<
          spt::util::json::str( model.document ) << ". " << magic_enum::enum_name( resp.error().cause ) << ". " << resp.error().message << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, std::format( "Database error. {}", resp.error().message ) );
        return { 417, std::nullopt };
      }

      if ( resp.value().results.empty() )
      {
        LOG_WARN << "No matching documents for query against " << M::Database() << ':' << M::Collection() << ". " << spt::util::json::str( model.document ) << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "No matching documents" );
        return { 404, std::nullopt };
      }

      return { 200, std::move( resp.value().results.front() ) };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error executing query against " << M::Database() << ':' << M::Collection() << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
      return { 422, std::nullopt };
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error executing query against " << M::Database() << ':' << M::Collection() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      p.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return { 500, std::nullopt };
  }

  template <Model M>
  std::tuple<int16_t, std::optional<model::Entities<M>>> pipeline( bsoncxx::document::value match,
      std::vector<spt::mongoservice::api::model::request::Pipeline::Document::Stage> stages,
      model::EntitiesQuery options, spt::ilp::APMRecord& apm, bool caseInsensitive = false )
  {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::finalize;

    auto& p = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    DEFER( spt::ilp::setDuration( p ) );

    try
    {
      auto req = spt::mongoservice::api::model::request::Pipeline{};
      req.database = M::Database();
      req.collection = M::Collection();
      req.correlationId = apm.id;
      req.document.specification = std::move( stages );
      if ( caseInsensitive )
      {
        req.options.emplace();
        req.options->collation.emplace();
        req.options->collation->locale = "en";
        req.options->collation->strength = 1;
      }

      auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Step );
      spt::ilp::addCurrentFunction( cp );
      auto resp = spt::mongoservice::api::repository::pipeline<M>( req );
      spt::ilp::setDuration( cp );

      if ( !resp.has_value() )
      {
        LOG_WARN << "Error executing query against " << M::Database() << ':' << M::Collection() << ". " <<
          spt::util::json::str( req.document ) << ". " << magic_enum::enum_name( resp.error().cause ) << ". " << resp.error().message << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, std::format( "Database error. {}", resp.error().message ) );
        return { 417, std::nullopt };
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
        auto idx = apm.processes.size();
        if ( const auto [cstatus, csize] = count<M>( match, apm ); cstatus == 200 ) ms.total = csize;
        spt::ilp::addCurrentFunction( apm.processes[idx] );
      }

      if ( ms.page > 0 && ms.total != ms.page )
      {
        auto idx = apm.processes.size();
        auto [lstatus, lv] = lastId<M>( match, document{} << "_id" << 1 << finalize,
          document{} << "_id" << (options.descending ? 1 : -1) << finalize, apm );
        spt::ilp::addCurrentFunction( apm.processes[idx] );
        if ( lstatus != 200 ) return { lstatus, std::nullopt };

        auto lid = ms.entities.back().id;
        if ( lid != *lv ) ms.next = lid.to_string();
      }

      return { 200, std::move( ms ) };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error executing query against " << M::Database() << ':' << M::Collection() << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
      return { 422, std::nullopt };
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error executing query against " << M::Database() << ':' << M::Collection() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      p.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return { 500, std::nullopt };
  }

  template <Model M>
  std::tuple<int16_t, std::optional<std::vector<M>>> rawpipeline(
    std::vector<spt::mongoservice::api::model::request::Pipeline::Document::Stage> stages, spt::ilp::APMRecord& apm )
  {
    auto& p = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    DEFER( spt::ilp::setDuration( p ) );

    try
    {
      auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Step );
      cp.values.try_emplace( model::ilp::name::APM_STEP_PROCESS, "retrieve data" );
      spt::ilp::addCurrentFunction( cp );

      auto req = spt::mongoservice::api::model::request::Pipeline{};
      req.database = M::Database();
      req.collection = M::Collection();
      req.correlationId = apm.id;
      req.document.specification = std::move( stages );

      auto res = spt::mongoservice::api::repository::pipeline<M>( req );
      if ( !res.has_value() )
      {
        LOG_WARN << "Error executing query against " << M::Database() << ':' << M::Collection() << ". " <<
          spt::util::json::str( req.document ) << ". " << magic_enum::enum_name( res.error().cause ) << ". " << res.error().message << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, std::format( "Database error. {}", res.error().message ) );

        return { 417, std::nullopt };
      }

      return { 200, std::move( res.value().results ) };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error executing query against " << M::Database() << ':' << M::Collection() << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
      return { 422, std::nullopt };
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error executing query against " << M::Database() << ':' << M::Collection() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      p.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return { 500, std::nullopt };
  }

  template <Model M>
  std::tuple<int16_t, std::optional<M>> retrieve( bsoncxx::oid id, std::string_view customer, spt::ilp::APMRecord& apm )
  {
    auto& p = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    DEFER( spt::ilp::setDuration( p ) );

    if ( auto pl = model::pipeline<M>(); !pl.empty() )
    {
      LOG_DEBUG << "Entity " << M::EntityType() << " requires pipeline. APM id: " << apm.id;
      auto pidx = apm.processes.size();

      auto filter = filter::Id{ customer, id };
      auto stages = std::vector<spt::mongoservice::api::model::request::Pipeline::Document::Stage>{};
      stages.reserve( pl.size() + 1 );

      stages.emplace_back( "$match", spt::util::bson( filter ) );
      stages.insert( stages.end(), std::make_move_iterator( pl.begin() ), std::make_move_iterator( pl.end() ) );
      pl.erase( pl.begin(), pl.end() );

      auto result = pipeline<M>( std::move( stages ), apm );
      spt::ilp::addCurrentFunction( apm.processes[pidx] );
      return result;
    }

    const auto ckey = cacheKey( M::EntityType(), id );
    {
      auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Step );
      cp.values.try_emplace( model::ilp::name::APM_STEP_PROCESS, "retrieve cached value" );
      spt::ilp::addCurrentFunction( cp );
      if ( auto cached = util::Configuration::instance().get( ckey ); cached )
      {
        auto mv = bsoncxx::document::view( reinterpret_cast<const uint8_t*>( cached->data() ), cached->size() );
        auto m = M{ mv };
        cp.values.try_emplace( model::ilp::name::APM_NOTE_VALUE, "Return cached value" );
        spt::ilp::setDuration( cp );
        return { 200, std::move( m ) };
      }
      spt::ilp::setDuration( cp );
    }

    try
    {
      auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Step );
      cp.values.try_emplace( model::ilp::name::APM_STEP_PROCESS, "retrieve data" );
      spt::ilp::addCurrentFunction( cp );

      auto r = spt::mongoservice::api::model::request::Retrieve<filter::Id>{};
      r.database = M::Database();
      r.collection = M::Collection();
      r.correlationId = apm.id;
      r.document.emplace( customer, id );

      auto resp = spt::mongoservice::api::repository::retrieve<M>( r );
      spt::ilp::setDuration( cp );
      if ( !resp.has_value() )
      {
        LOG_WARN << "Error retrieving document " << M::Database() << ':' << M::Collection() << ':' << id.to_string() <<
           ". " << magic_enum::enum_name( resp.error().cause ) << ". " << resp.error().message << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, std::format( "Database error. {}", resp.error().message ) );
        return { resp.error().message == "Not found" ? 404 : 417, std::nullopt };
      }

      if ( !resp.value().result )
      {
        LOG_WARN << "No document " << M::Database() << ':' << M::Collection() << ':' << id.to_string() << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Document not found" );
        return { 404, std::nullopt };
      }

      {
        auto& cache = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Step );
        DEFER( spt::ilp::setDuration( cache ) );
        cache.values.try_emplace( model::ilp::name::APM_STEP_PROCESS, "cache data" );
        spt::ilp::addCurrentFunction( cache );
        auto bson = spt::util::marshall( *resp.value().result );
        auto dv = bson.view();
        auto str = std::string{ reinterpret_cast<const char *>( dv.data() ), dv.length() };
        util::Configuration::instance().set( ckey, str, 600 );
      }

      return { 200, std::move( resp.value().result ) };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error retrieving document " << M::Database() << ':' << M::Collection() << ':' << id.to_string() << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
      return { 422, std::nullopt };
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error retrieving document " << M::Database() << ':' << M::Collection() << ':' << id.to_string() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      p.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return { 500, std::nullopt };
  }

  template<Model M, typename ValueType>
  std::tuple<int16_t, std::optional<M>> retrieve( std::string_view property, ValueType value,
      std::string_view customer, spt::ilp::APMRecord& apm, bool caseInsensitive = false )
  {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::finalize;

    auto& p = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    DEFER( spt::ilp::setDuration( p ) );

    if ( auto pl = model::pipeline<M>(); !pl.empty() )
    {
      auto pidx = apm.processes.size();
      LOG_DEBUG << "Entity " << M::EntityType() << " requires pipeline. APM id: " << apm.id;
      auto stages = std::vector<spt::mongoservice::api::model::request::Pipeline::Document::Stage>{};
      stages.reserve( 3 + pl.size() );

      auto match = filter::Property<ValueType>{ property, value, customer };
      stages.emplace_back( "$match", spt::util::bson( match ) );
      stages.emplace_back( "$sort", document{} << "_id" << -1 << finalize );
      stages.emplace_back( "$limit", bsoncxx::types::b_int32{ 1 } );

      stages.insert( stages.end(), std::make_move_iterator( pl.begin() ), std::make_move_iterator( pl.end() ) );
      pl.erase( pl.begin(), pl.end() );

      auto result = pipeline<M>( std::move( stages ), apm, caseInsensitive );
      spt::ilp::addCurrentFunction( apm.processes[pidx] );
      return result;
    }

    try
    {
      auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Step );
      cp.values.try_emplace( model::ilp::name::APM_STEP_PROCESS, "retrieve data" );
      spt::ilp::addCurrentFunction( cp );

      auto r = spt::mongoservice::api::model::request::Retrieve<filter::Property<ValueType>>{};
      r.database = M::Database();
      r.collection = M::Collection();
      r.correlationId = apm.id;
      r.document.emplace( property, value, customer );

      r.options.emplace();
      r.options->limit = 1;
      r.options->sort = document{} << "_id" << -1 << finalize;
      if ( caseInsensitive )
      {
        r.options->collation.emplace();
        r.options->collation->locale = "en";
        r.options->collation->strength = 1;
      }

      auto resp = spt::mongoservice::api::repository::retrieve<M>( r );
      spt::ilp::setDuration( cp );
      if ( !resp.has_value() )
      {
        LOG_WARN << "Unable to retrieve document from " << M::Database() << ':' << M::Collection() <<
          " with property: " << property << ", value: " << value <<
          ". " << magic_enum::enum_name( resp.error().cause ) << ". " << resp.error().message << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, std::format( "Database error. {}", resp.error().message ) );

        return { resp.error().message == "Not found" ? 404 : 417, std::nullopt };
      }

      if ( resp.value().results.empty() )
      {
        LOG_WARN << "No matching document " << M::Database() << ':' << M::Collection() <<
          " with property: " << property << ", value: " << value << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Document not found" );

        return { 404, std::nullopt };
      }

      return { 200, std::move( resp.value().results.front() ) };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error retrieving document " << M::Database() << ':' << M::Collection() <<
          " with property: " << property << ", value: " << value << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
      return { 422, std::nullopt };
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error retrieving document " << M::Database() << ':' << M::Collection() <<
          " with property: " << property << ", value: " << value << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      p.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return { 500, std::nullopt };
  }

  template <Model M>
  std::tuple<int16_t, std::optional<model::Entities<M>>> query( bsoncxx::document::value query,
    const model::EntitiesQuery& options, spt::ilp::APMRecord& apm )
  {
    using bsoncxx::builder::stream::array;
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::finalize;

    auto& p = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    DEFER( spt::ilp::setDuration( p ) );

    if ( auto pl = model::pipeline<M>(); !pl.empty() )
    {
      LOG_DEBUG << "Entity " << M::EntityType() << " requires pipeline. APM id: " << apm.id;
      auto pidx = apm.processes.size();

      auto stages = std::vector<spt::mongoservice::api::model::request::Pipeline::Document::Stage>{};
      stages.reserve( pl.size() + 3 );
      stages.emplace_back( "$match", query.view() );
      stages.emplace_back( "$sort", document{} << "_id" << (options.descending ? -1 : 1) << finalize );
      stages.emplace_back( "$limit", bsoncxx::types::b_int32{ 1 } );

      stages.insert( stages.end(), std::make_move_iterator( pl.begin() ), std::make_move_iterator( pl.end() ) );
      pl.erase( pl.begin(), pl.end() );

      auto result = pipeline<M>( std::move( query ), std::move( stages ), options, apm );
      spt::ilp::addCurrentFunction( apm.processes[pidx] );
      return result;
    }

    try
    {
      auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Step );
      cp.values.try_emplace( model::ilp::name::APM_STEP_PROCESS, "retrieve data" );
      spt::ilp::addCurrentFunction( cp );

      auto retrieve = spt::mongoservice::api::model::request::Retrieve{ bsoncxx::document::value{ query } };
      retrieve.database = M::Database();
      retrieve.collection = M::Collection();
      retrieve.correlationId = apm.id;
      retrieve.options.emplace();
      retrieve.options->limit = options.limit;
      retrieve.options->sort = document{} << "_id" << (options.descending ? -1 : 1) << finalize;

      auto result = spt::mongoservice::api::repository::retrieve<M>( retrieve );
      spt::ilp::setDuration( cp );
      if ( !result.has_value() )
      {
        LOG_WARN << "Unable to execute query against " << M::Database() << ':' << M::Collection() << ". " <<
          magic_enum::enum_name( result.error().cause ) << ". " << result.error().message << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, std::format( "Database error. {}", result.error().message ) );
        return { 417, std::nullopt };
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
        auto pidx = apm.processes.size();
        if ( const auto [cstatus, csize] = count<M>( query, apm ); cstatus == 200 ) ms.total = csize;
        spt::ilp::addCurrentFunction( apm.processes[pidx] );
      }

      if ( ms.page > 0 && ms.total != ms.page )
      {
        auto pidx = apm.processes.size();
        auto [lstatus, lv] = lastId<M>( query, document{} << "_id" << 1 << finalize,
          document{} << "_id" << (options.descending ? 1 : -1) << finalize, apm );
        spt::ilp::addCurrentFunction( apm.processes[pidx] );
        if ( lstatus != 200 )
        {
          cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Error retrieving last id" );
          return { lstatus, std::nullopt };
        }

        auto lid = ms.entities.back().id;
        if ( lid != *lv ) ms.next = lid.to_string();
      }

      return { 200, std::move( ms ) };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error executing query against " << M::Database() << ':' << M::Collection() << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
      return { 422, std::nullopt };
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error executing query against " << M::Database() << ':' << M::Collection() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      p.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return { 500, std::nullopt };
  }

  template <Model M>
  std::tuple<int16_t, std::optional<std::vector<M>>> rawquery( bsoncxx::document::value query,
    const model::EntitiesQuery& options, spt::ilp::APMRecord& apm, bool caseInsensitive = false )
  {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::finalize;

    auto& p = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    DEFER( spt::ilp::setDuration( p ) );

    try
    {
      auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Step );
      cp.values.try_emplace( model::ilp::name::APM_STEP_PROCESS, "retrieve data" );
      spt::ilp::addCurrentFunction( cp );

      auto retrieve = spt::mongoservice::api::model::request::Retrieve{ bsoncxx::document::value{ query } };
      retrieve.database = M::Database();
      retrieve.collection = M::Collection();
      retrieve.correlationId = apm.id;
      retrieve.options.emplace();
      retrieve.options->limit = options.limit;
      retrieve.options->sort = document{} << "_id" << (options.descending ? -1 : 1) << finalize;
      if ( caseInsensitive )
      {
        retrieve.options->collation.emplace();
        retrieve.options->collation->locale = "en";
        retrieve.options->collation->strength = 1;
      }

      auto result = spt::mongoservice::api::repository::retrieve<M>( retrieve );
      spt::ilp::setDuration( cp );
      if ( !result.has_value() )
      {
        LOG_WARN << "Unable to execute query against " << M::Database() << ':' << M::Collection() << ". " <<
          magic_enum::enum_name( result.error().cause ) << ". " << result.error().message << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, std::format( "Database error. {}", result.error().message ) );

        return { 417, std::nullopt };
      }

      return { 200, std::move( result.value().results ) };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error executing query against " << M::Database() << ':' << M::Collection() << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
      return { 422, std::nullopt };
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error executing query against " << M::Database() << ':' << M::Collection() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      p.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return { 500, std::nullopt };
  }

  template<Model M, typename ValueType>
  std::tuple<int16_t, std::optional<std::vector<M>>> rawquery( std::string_view property, ValueType value,
      std::string_view customer, const model::EntitiesQuery& options, spt::ilp::APMRecord& apm, bool caseInsensitive = false )
  {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::finalize;

    auto& p = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    DEFER( spt::ilp::setDuration( p ) );

    auto f = filter::Property<ValueType>{ property, value, customer };

    if ( auto pl = model::pipeline<M>(); !pl.empty() )
    {
      LOG_DEBUG << "Entity " << M::EntityType() << " requires pipeline. APM id: " << apm.id;
      auto pidx = apm.processes.size();

      auto stages = std::vector<spt::mongoservice::api::model::request::Pipeline::Document::Stage>{};
      stages.reserve( pl.size() + 3 );
      stages.emplace_back( "$match", spt::util::marshall( f ) );
      stages.emplace_back( "$sort", document{} << "_id" << (options.descending ? -1 : 1) << finalize );
      stages.emplace_back( "$limit", bsoncxx::types::b_int32{ options.limit } );

      stages.insert( stages.end(), std::make_move_iterator( pl.begin() ), std::make_move_iterator( pl.end() ) );
      pl.erase( pl.begin(), pl.end() );

      auto result = rawpipeline<M>( std::move( stages ), apm );
      spt::ilp::addCurrentFunction( apm.processes[pidx] );
      return result;
    }

    return rawquery<M>( spt::util::marshall( f ), options, apm, caseInsensitive );
  }

  template <Model M>
  int16_t transaction( bsoncxx::array::view items, spt::ilp::APMRecord& apm )
  {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::finalize;
    using spt::mongoservice::api::execute;
    using spt::mongoservice::api::Request;

    auto& p = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    DEFER( spt::ilp::setDuration( p ) );

    try
    {
      auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Step );
      cp.values.try_emplace( model::ilp::name::APM_STEP_PROCESS, "retrieve data" );
      spt::ilp::addCurrentFunction( cp );

      auto req = Request::transaction( std::string{ M::Database() }, std::string{ M::Collection() },
          document{} << "items" << items << finalize );
      req.correlationId = apm.id;
      const auto [type, opt] = execute( req );
      spt::ilp::setDuration( cp );
      if ( !opt )
      {
        LOG_WARN << "Unable to execute transaction against " << M::Database() << ':' << M::Collection() << ". " << req.document << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Database error." );
        return 424;
      }

      const auto view = opt->view();
      if ( const auto err = spt::util::bsonValueIfExists<std::string>( "error", view ); err )
      {
        LOG_WARN << "Error executing query against " << M::Database() << ':' << M::Collection() << ". " << req.document << ". " << view << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, std::format( "Error executing query. {}", *err ) );
        return 417;
      }

      LOG_INFO << "Transaction results " << view << ". APM id: " << apm.id;
      return 200;
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error executing query against " << M::Database() << ':' << M::Collection() << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
      return 422;
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error executing query against " << M::Database() << ':' << M::Collection() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      p.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return 500;
  }

  template <Model M>
  std::tuple<int16_t, std::optional<model::Entities<M>>> between( const filter::Between& filter,
    model::EntitiesQuery options, spt::ilp::APMRecord& apm )
  {
    using bsoncxx::builder::stream::array;
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::finalize;
    using spt::mongoservice::api::execute;
    using spt::mongoservice::api::Request;

    auto& p = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    DEFER( spt::ilp::setDuration( p ) );

    if ( auto pl = model::pipeline<M>(); !pl.empty() )
    {
      LOG_DEBUG << "Entity " << M::EntityType() << " requires pipeline. APM id: " << apm.id;

      auto idx = apm.processes.size();
      auto stages = std::vector<spt::mongoservice::api::model::request::Pipeline::Document::Stage>{};
      stages.reserve( pl.size() + 3 );
      stages.emplace_back( "$match", spt::util::bson( filter ) );
      stages.emplace_back( "$sort", document{} << filter.field << (options.descending ? -1 : 1) << finalize );
      stages.emplace_back( "$limit", bsoncxx::types::b_int32{ options.limit } );

      stages.insert( stages.end(), std::make_move_iterator( pl.begin() ), std::make_move_iterator( pl.end() ) );
      pl.erase( pl.begin(), pl.end() );

      auto result = pipeline<M>( spt::util::marshall( filter ), std::move( stages ), options, apm );
      spt::ilp::addCurrentFunction( apm.processes[idx] );
      return result;
    }

    try
    {
      auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Step );
      cp.values.try_emplace( model::ilp::name::APM_STEP_PROCESS, "retrieve data" );
      spt::ilp::addCurrentFunction( cp );

      auto retrieve = spt::mongoservice::api::model::request::Retrieve{ spt::util::marshall( filter ) };
      retrieve.database = M::Database();
      retrieve.collection = M::Collection();
      retrieve.correlationId = apm.id;
      retrieve.options.emplace();
      retrieve.options->limit = options.limit;
      retrieve.options->sort = document{} << filter.field << (options.descending ? -1 : 1) << finalize;
      auto res = spt::mongoservice::api::repository::retrieve<M>( retrieve );
      spt::ilp::setDuration( cp );

      if ( !res.has_value() )
      {
        LOG_WARN << "Error executing query against " << M::Database() << ':' << M::Collection() << ". " <<
          magic_enum::enum_name( res.error().cause ) << ". " << res.error().message << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, std::format( "Database error. {}", res.error().message ) );
        return { 417, std::nullopt };
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
        auto idx = apm.processes.size();
        if ( const auto [cstatus, csize] = count<M>( *retrieve.document, apm ); cstatus == 200 ) ms.total = csize;
        spt::ilp::addCurrentFunction( apm.processes[idx] );
      }

      if ( ms.page > 0 && ms.total != ms.page )
      {
        auto idx = apm.processes.size();
        auto [lstatus, lv] = lastId<M>( *retrieve.document,
          document{} << filter.field << 1 << finalize,
          document{} << filter.field << (options.descending ? 1 : -1) << finalize, apm );
        spt::ilp::addCurrentFunction( apm.processes[idx] );
        if ( lstatus != 200 )
        {
          cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Error retrieving last id" );
          return { lstatus, std::nullopt };
        }

        if ( ms.entities.back().id != *lv )
        {
          ms.next = spt::util::isoDateMillis( filter.field == "created"sv ? ms.entities.back().metadata.created : ms.entities.back().metadata.modified );
        }
      }

      return { 200, std::move( ms ) };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error executing query against " << M::Database() << ':' << M::Collection() << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
      return { 422, std::nullopt };
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error executing query against " << M::Database() << ':' << M::Collection() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      p.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return { 500, std::nullopt };
  }

  template <Model M>
  std::tuple<int16_t, std::optional<model::RefCounts>> refcounts( bsoncxx::oid id, spt::ilp::APMRecord& apm )
  {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::finalize;
    using spt::mongoservice::api::execute;
    using spt::mongoservice::api::Request;

    auto& p = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    DEFER( spt::ilp::setDuration( p ) );

    const auto refs = M::References();
    if ( refs.empty() ) return { 200, std::nullopt };

    try
    {
      model::RefCounts rf;
      rf.references.reserve( refs.size() );

      for ( const auto& ref : refs )
      {
        auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Step );
        cp.values.try_emplace( model::ilp::name::APM_STEP_PROCESS, "retrieve data" );
        spt::ilp::addCurrentFunction( cp );

        auto req = spt::mongoservice::api::model::request::Count<bsoncxx::document::value>{ document{} << ref.field << id << finalize };
        req.database = model::database( ref.type );
        req.collection = model::collection( ref.type );
        req.correlationId = apm.id;

        const auto resp = spt::mongoservice::api::repository::count( req );
        spt::ilp::setDuration( cp );
        if ( !resp.has_value() )
        {
          LOG_WARN << "Error refcounting document " << M::Database() << ':' << M::Collection() << ':' << id.to_string() <<
            " in collection " << model::collection( ref.type ) << ". " <<
              magic_enum::enum_name( resp.error().cause ) << ". " << resp.error().message << ". APM id: " << apm.id;
          cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, std::format( "Database error. {}", resp.error().message ) );

          return { 417, std::nullopt };
        }

        rf.references.emplace_back( model::RefCount{ static_cast<int32_t>( resp.value().count ), ref.type } );
      }

      return { 200, std::move( rf ) };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error retrieving reference counts for " << M::Database() << ':' << M::Collection() << ':' << id << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
      return { 422, std::nullopt };
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error retrieving document " << M::Database() << ':' << M::Collection() << ':' << id << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      p.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return { 500, std::nullopt };
  }

  template <Model M>
  int16_t remove( bsoncxx::oid id, std::string_view customer, spt::ilp::APMRecord& apm )
  {
    auto& p = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    DEFER( spt::ilp::setDuration( p ) );

    try
    {
      auto pidx = apm.processes.size();
      auto [rstatus, rc] = refcounts<M>( id, apm );
      spt::ilp::addCurrentFunction( apm.processes[pidx] );
      if ( rstatus != 200 ) return rstatus;
      if ( rc )
      {
        for ( const auto& r : rc->references )
        {
          if ( r.count > 0 )
          {
            LOG_INFO << "Rejecting delete document " << M::Database() << ':' <<
              M::Collection() << ':' << id.to_string() <<
              " as it is being referenced. " << spt::util::json::str( *rc ) << ". APM id: " << apm.id;
            p.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Cannot deleted entity that is being referenced" );
            return 412;
          }
        }
      }

      auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Step );
      cp.values.try_emplace( model::ilp::name::APM_STEP_PROCESS, "remove data" );
      spt::ilp::addCurrentFunction( cp );

      auto now = std::chrono::system_clock::now();
      auto req = spt::mongoservice::api::model::request::Delete<filter::Id, Metadata>( filter::Id{} );
      req.database = M::Database();
      req.collection = M::Collection();
      req.correlationId = apm.id;
      req.document->id = id;
      if ( !customer.empty() ) req.document->customer = customer;
      req.metadata.emplace();
      req.metadata->year = std::format( "{:%Y}", now );
      req.metadata->month = std::format( "{:%m}", now );

      auto resp = spt::mongoservice::api::repository::remove( req );
      spt::ilp::setDuration( cp );

      if ( !resp.has_value() )
      {
        LOG_WARN << "Error deleting document " << M::Database() << ':' << M::Collection() << ':' << id.to_string() << ". " <<
          magic_enum::enum_name( resp.error().cause ) << ". " << resp.error().message << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, std::format( "Database error. {}", resp.error().message ) );

        return 417;
      }

      auto iter = ranges::find_if( resp.value().success, [&id]( const bsoncxx::oid& oid ) { return oid == id; } );
      if ( iter == ranges::end( resp.value().success ) )
      {
        LOG_WARN << "No document deleted for " << M::Database() << ':' << M::Collection() << ':' << id.to_string() << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "No document deleted" );

        return 417;
      }

      {
        auto& dc = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Step );
        dc.values.try_emplace( model::ilp::name::APM_STEP_PROCESS, "remove cache entry" );
        spt::ilp::addCurrentFunction( dc );
        util::Configuration::instance().remove( cacheKey( M::EntityType(), id ) );
        spt::ilp::setDuration( dc );
      }

      return 200;
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error removing document " << M::Database() << ':' << M::Collection() << ':' << id.to_string() <<
        ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
      return 422;
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error removing document " << M::Database() << ':' << M::Collection() << ':' << id.to_string() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      p.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return 500;
  }

  template <Model M>
  int16_t remove( bsoncxx::document::value&& query, spt::ilp::APMRecord& apm )
  {
    auto& p = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    DEFER( spt::ilp::setDuration( p ) );

    try
    {
      auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Step );
      cp.values.try_emplace( model::ilp::name::APM_STEP_PROCESS, "remove data" );
      spt::ilp::addCurrentFunction( cp );

      const auto now = std::chrono::system_clock::now();
      auto req = spt::mongoservice::api::model::request::Delete<bsoncxx::document::value, Metadata>{ std::move( query )  };
      req.database = M::Database();
      req.collection = M::Collection();
      req.correlationId = apm.id;
      req.metadata.emplace();
      req.metadata->year = std::format( "{:%Y}", now );
      req.metadata->month = std::format( "{:%m}", now );

      auto resp = spt::mongoservice::api::repository::remove( req );
      spt::ilp::setDuration( cp );
      if ( !resp.has_value() )
      {
        LOG_WARN << "Error deleting document(s) in " << M::Database() << ':' << M::Collection() << ". " <<
          bsoncxx::to_json( req.document->view() ) << ". " <<
          magic_enum::enum_name( resp.error().cause ) << ". " << resp.error().message << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, std::format( "Database error. {}", resp.error().message ) );

        return 417;
      }

      if ( resp.value().success.empty() )
      {
        LOG_WARN << "No documents deleted for " << M::Database() << ':' << M::Collection() << bsoncxx::to_json( req.document->view() ) << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "No documents deleted" );

        return 204;
      }

      return 200;
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error removing document(s) in " << M::Database() << ':' << M::Collection() << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
      return 422;
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error removing document(s) in " << M::Database() << ':' << M::Collection() <<
      LOG_WARN << util::stacktrace() << ". APM id: " << apm.id;
      p.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return 500;
  }

  template <Model M>
  std::tuple<int16_t, std::optional<model::Entities<model::VersionHistorySummary>>>
      versionHistorySummary( bsoncxx::oid id, spt::ilp::APMRecord& apm )
  {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::finalize;
    using spt::mongoservice::api::execute;
    using spt::mongoservice::api::Request;

    auto& p = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    DEFER( spt::ilp::setDuration( p ) );

    const auto& dbc = impl::DatabaseConfiguration::instance();

    try
    {
      auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Step );
      cp.values.try_emplace( model::ilp::name::APM_STEP_PROCESS, "remove data" );
      spt::ilp::addCurrentFunction( cp );

      auto req = Request::retrieve( dbc.historyDb, std::string{ model::VersionHistorySummary::Collection() },
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
      req.correlationId = apm.id;

      const auto [type, opt] = execute( req );
      spt::ilp::setDuration( cp );
      if ( !opt )
      {
        LOG_WARN << "Unable to retrieve version history summary documents for " <<
          M::Database() << ':' << M::Collection() << ':' << id.to_string() << ". " << req.document << ". " << *req.options << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Database error." );

        return { 424, std::nullopt };
      }

      const auto view = opt->view();
      if ( const auto err = spt::util::bsonValueIfExists<std::string>( "error", view ); err )
      {
        LOG_WARN << "Error retrieving version history summary documents for " <<
          M::Database() << ':' << M::Collection() << ':' << id <<
          ". " << req.document << ". " << *req.options << ". " << view << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, std::format( "Error returned. {}", *err ) );

        return { 417, std::nullopt };
      }

      auto arr = spt::util::bsonValueIfExists<bsoncxx::array::view>( "results", view );
      if ( !arr )
      {
        LOG_WARN << "No version history summary documents for " <<
          M::Database() << ':' << M::Collection() << ':' << id.to_string() <<
          ". " << req.document << ". " << *req.options << ". " << view << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "No matches returned." );

        return { 404, std::nullopt };
      }

      model::Entities<model::VersionHistorySummary> ms;
      ms.entities.reserve( std::distance( arr->begin(), arr->end() ) );
      std::for_each( std::cbegin( *arr ), std::cend( *arr ),
          [&ms]( const auto& d ) { ms.entities.emplace_back( d.get_document().value ); } );
      ms.page = std::size( ms.entities );
      ms.total = ms.page;

      return { 200, std::move( ms ) };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error retrieving version history summary documents for " <<
        M::Database() << ':' << M::Collection() << ':' << id.to_string() << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
      return { 422, std::nullopt };
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error retrieving version history documents for " <<
        M::Database() << ':' << M::Collection() << ':' << id.to_string() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      p.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return { 500, std::nullopt };
  }

  template <Model M>
  std::tuple<int16_t, std::optional<model::VersionHistoryDocument<M>>>
  versionHistoryDocument( bsoncxx::oid id, spt::ilp::APMRecord& apm )
  {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_document;
    using bsoncxx::builder::stream::close_document;
    using bsoncxx::builder::stream::finalize;
    using spt::mongoservice::api::execute;
    using spt::mongoservice::api::Request;

    auto& p = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Function );
    DEFER( spt::ilp::setDuration( p ) );

    const auto& dbc = impl::DatabaseConfiguration::instance();

    try
    {
      auto& cp = spt::ilp::addProcess( apm, spt::ilp::APMRecord::Process::Type::Step );
      cp.values.try_emplace( model::ilp::name::APM_STEP_PROCESS, "remove data" );
      spt::ilp::addCurrentFunction( cp );

      auto req = Request::retrieve( dbc.historyDb, std::string{ model::VersionHistorySummary::Collection() },
          document{} <<
            "_id" << id <<
            "database" << M::Database() <<
            "collection" << M::Collection() <<
          finalize );
      req.correlationId = apm.id;
      const auto [type, opt] = execute( req );
      spt::ilp::setDuration( cp );
      if ( !opt )
      {
        LOG_WARN << "Unable to retrieve version history document for " <<
          M::Database() << ':' << M::Collection() << " with id: " << id << ". " << req.document << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Database error." );

        return { 424, std::nullopt };
      }

      const auto view = opt->view();
      if ( const auto err = spt::util::bsonValueIfExists<std::string>( "error", view ); err )
      {
        LOG_WARN << "Error retrieving version history document for " <<
          M::Database() << ':' << M::Collection() << " with id: " << id << ". " << req.document << ". " << view << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, std::format( "Error returned {}.", *err ) );

        return { 417, std::nullopt };
      }

      auto doc = spt::util::bsonValueIfExists<bsoncxx::document::view>( "result", view );
      if ( !doc )
      {
        LOG_WARN << "No version history document for " <<
          M::Database() << ':' << M::Collection() << " with id: " << id << ". " << req.document << ". " << view << ". APM id: " << apm.id;
        cp.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "No matches returned." );

        return { 404, std::nullopt };
      }

      return { 200, model::VersionHistoryDocument<M>{ *doc } };
    }
    catch ( const std::exception& ex )
    {
      LOG_WARN << "Error retrieving version history document for " <<
        M::Database() << ':' << M::Collection() << " with id: " << id.to_string() << ". " << ex.what() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      spt::ilp::addException( apm, ex, "Exception" );
      return { 422, std::nullopt };
    }
    catch ( ... )
    {
      LOG_WARN << "Unknown error retrieving version history document for " <<
        M::Database() << ':' << M::Collection() << " with id: " << id.to_string() << ". APM id: " << apm.id;
      LOG_WARN << util::stacktrace();
      p.values.try_emplace( model::ilp::name::APM_ERROR_VALUE, "Unexpected error" );
    }

    return { 500, std::nullopt };
  }
}

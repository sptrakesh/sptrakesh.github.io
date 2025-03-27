//
// Created by Rakesh on 6/4/21.
//

#include "db/repository.hpp"
#include "http/response.hpp"
#include "util/config.hpp"

#include <charconv>
#include <boost/json/parse.hpp>
#include <fmt/ranges.h>
#include <http2/framework/common.hpp>
#include <log/NanoLog.hpp>
#include <mongo-service/api/api.hpp>
#include <range/v3/algorithm/find.hpp>

using spt::http::Response;
using std::operator""s;
using std::operator""sv;

namespace
{
  namespace presponse
  {
    std::vector<std::string> origins()
    {
      const auto def = []()
      {
        return std::vector{
            "http://local.sptci.com:3000"s,
            "https://sso-dev.sptci.com"s, "https://sso.sptci.com"s,
            "https://dashboard-dev.sptci.com"s, "https://admin-dev.sptci.com"s,
            "https://dashboard.sptci.com"s, "https://admin.sptci.com"s,
            "https://www.sptci.com"s, "https://sptci.com"s
        };
      };

      if ( auto cfg = spt::util::Configuration::instance().get( "/service/cors/origins"sv ); cfg )
      {
        auto ec = boost::system::error_code{};
        auto p = boost::json::parse( *cfg, ec );

        if ( ec )
        {
          LOG_CRIT << "Error parsing configured origins " << *cfg;
          return def();
        }

        auto resp = std::vector<std::string>{};
        resp.reserve( 8 );

        for ( const auto& item : p.as_array() ) resp.emplace_back( item.as_string() );
        LOG_INFO << "Configured origins " << fmt::format( "{}", resp );
        return resp;
      }

      LOG_WARN << "Configured origins not found.  Using defaults...";
      return def();
    }

    template<class... Ts> struct overload : Ts... { using Ts::operator()...; };

    void serialise( const std::map<std::string, spt::ilp::APMRecord::Value, std::less<>>& map,
      bsoncxx::builder::stream::document& builder, std::string_view property )
    {
      using bsoncxx::builder::stream::document;
      using bsoncxx::builder::stream::finalize;

      auto values = document{};
      for ( const auto& pair : map )
      {
        std::visit( overload
          {
            [&values, &pair]( bool v ){ values << pair.first << v; },
            [&values, &pair]( int64_t v ){ values << pair.first << v; },
            [&values, &pair]( uint64_t v ){ values << pair.first << static_cast<int64_t>( v ); },
            [&values, &pair]( double v ){ values << pair.first << v; },
            [&values, &pair]( const std::string& v ){ values << pair.first << v; },
          }, pair.second );
      }
      builder << property << (values << finalize);
    }

    bsoncxx::document::value serialise( const spt::ilp::APMRecord::Process& process )
    {
      using bsoncxx::builder::stream::document;
      using bsoncxx::builder::stream::open_document;
      using bsoncxx::builder::stream::close_document;
      using bsoncxx::builder::stream::finalize;

      auto doc = document{};
      doc << "type" << magic_enum::enum_name( process.type ) <<
        "timestamp" << bsoncxx::types::b_int64{ std::chrono::duration_cast<std::chrono::nanoseconds>( process.timestamp.time_since_epoch() ).count() } <<
        "duration" << bsoncxx::types::b_int64{ process.duration.count() };

      auto tags = document{};
      for ( const auto& [key, value] : process.tags ) tags << key << value;
      doc << "tags" << (tags << finalize);

      serialise( process.values, doc, "values" );

      return doc << finalize;
    }

    bsoncxx::document::value serialise( const spt::ilp::APMRecord& apm )
    {
      using bsoncxx::builder::stream::array;
      using bsoncxx::builder::stream::document;
      using bsoncxx::builder::stream::finalize;

      auto doc = document{};
      doc << "_id" << bsoncxx::oid{ apm.id } <<
        "application" << apm.application <<
        "timestamp" << bsoncxx::types::b_int64{ std::chrono::duration_cast<std::chrono::nanoseconds>( apm.timestamp.time_since_epoch() ).count() } <<
        "duration" << bsoncxx::types::b_int64{ apm.duration.count() };

      auto tags = document{};
      for ( const auto& [key, value] : apm.tags ) tags << key << value;
      doc << "tags" << (tags << finalize);

      serialise( apm.values, doc, "values" );

      auto arr = array{};
      for ( const auto& proc : apm.processes ) arr << serialise( proc );
      doc << "processes" << (arr << finalize);

      return doc << finalize;
    }
  }
}

const std::vector<std::string>& Response::origins()
{
  static const auto vec = presponse::origins();
  return vec;
}

Response::Response( const nghttp2::asio_http2::header_map& headers )
{
  auto iter = headers.find( "origin"s );
  if ( iter == std::cend( headers ) ) iter = headers.find( "Origin"s );
  if ( iter == std::cend( headers ) ) return;
  origin = iter->second.value;
}

void Response::set( std::span<const std::string> methods, std::span<const std::string> origins )
{
  headers = nghttp2::asio_http2::header_map{
      { "Access-Control-Allow-Methods", { fmt::format( "{:n:}", methods ), false } },
      { "Access-Control-Allow-Headers", { "*, authorization", false } },
      { "content-type", { "application/json; charset=utf-8", false } },
      { "content-length", { std::to_string( body.size() ), false } }
  };
  if ( compressed )
  {
    headers.emplace( "content-encoding", nghttp2::asio_http2::header_value{ "gzip", false } );
  }

  if ( origin.empty() ) return;

  if ( const auto iter = ranges::find( origins, origin ); iter != ranges::end( origins ) )
  {
    headers.emplace( "Access-Control-Allow-Origin", nghttp2::asio_http2::header_value{ origin, false } );
    headers.emplace( "Vary", nghttp2::asio_http2::header_value{ "Origin", false } );
  }
  else LOG_WARN << "Origin " << origin << " not allowed";
}

template <>
void spt::http2::framework::extraProcess( const Request& req, spt::http::Response& response, boost::asio::thread_pool& pool )
{
  if ( !response.apm ) return;

  int32_t size{ 0 };
  if ( response.filePath.empty() )
  {
    size = static_cast<int32_t>( response.body.size() );
  }
  else
  {
    if ( const auto iter = response.headers.find( "content-length"s ); iter != std::cend( response.headers ) )
    {
      std::from_chars( iter->second.value.data(), iter->second.value.data() + iter->second.value.size(), size );
    }
  }

  response.apm->tags.try_emplace( "request_method", req.method );
  response.apm->tags.try_emplace( "entity", response.entity );
  response.apm->tags.try_emplace( "host", spt::util::hostname() );
  response.apm->tags.try_emplace( "response_status", std::to_string( response.status ) );
  response.apm->tags.try_emplace( "response_compressed", std::format( "{}", response.compressed ) );

  response.apm->values.try_emplace( "request_path", req.path );
  response.apm->values.try_emplace( "ipaddress",  ipaddress( req ) );
  response.apm->values.try_emplace( "response_size", static_cast<int64_t>( size ) );

  if ( response.jwt )
  {
    response.apm->values.try_emplace( "username", response.jwt->user.username );
    response.apm->tags.try_emplace( "customer", response.jwt->user.customerCode );
    response.apm->tags.try_emplace( "role", std::string{ magic_enum::enum_name( response.jwt->user.role ) } );
  }

  spt::ilp::setDuration( *response.apm );

  boost::asio::post( pool, [apm=std::move(response.apm.value())]
  {
    auto request = mongoservice::api::Request::create( spt::model::database( spt::model::EntityType::SearchDocument ),
      "webapm", presponse::serialise( apm ) );
    request.correlationId = apm.id;
    request.skipVersion = true;

    const auto [_, opt] = mongoservice::api::execute( request );
    if ( !opt )
    {
      LOG_WARN << "Unable to save apm " << request.database << ':' << request.collection << ':' << apm.id;
      return;
    }

    const auto view = opt->view();
    if ( const auto err = util::bsonValueIfExists<std::string>( "error", view ); err )
    {
      LOG_WARN << "Unable to save apm " << request.database << ':' << request.collection << ':' << apm.id << ". " << *err;
      return;
    }

    LOG_DEBUG << "Saved apm " << request.database << ':' << request.collection << ':' << apm.id;
  } );
}

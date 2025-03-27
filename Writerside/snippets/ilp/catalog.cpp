//
// Created by Rakesh on 22/08/2024.
//

#include "http/template.hpp"
#include "model/catalog.hpp"

using std::operator ""s;
using std::operator ""sv;
using spt::http2::framework::RoutingRequest;

#define CREATE_APM() \
    auto apm = spt::ilp::createAPMRecord( bsoncxx::oid{}.to_string(), "inventory-api", spt::ilp::APMRecord::Process::Type::Function, 48 );

#define FINISH_APM() \
  spt::ilp::addCurrentFunction( apm.processes[1] ); \
  resp.apm = std::move( apm ); \
  return resp;
}

void spt::http::addCatalogRoutes( spt::http2::framework::Server<Response>& server )
{
  static const auto methods = std::array{ "GET"s, "OPTIONS"s, "POST"s, "PUT"s, "DELETE"s };

  server.addHandler( "GET"sv, "/catalog/"sv, []( const RoutingRequest& req, auto&& )
  {
    CREATE_APM()
    auto resp = retrieveAll<model::Catalog>( req.req, methods, apm, &userRole );
    FINISH_APM()
  }, "./paths/catalog.yaml#/root"sv );

  server.addHandler( "GET"sv, "/catalog/id/{id}"sv, []( const RoutingRequest& req, auto args )
  {
    CREATE_APM()
    auto resp = get<model::Catalog>( req.req, args["id"sv], methods, apm, &userRole );
    FINISH_APM()
  }, "./paths/catalog.yaml#/id"sv );

  server.addHandler( "GET"sv, "/catalog/identifier/{identifier}"sv, []( const RoutingRequest& req, auto args )
  {
    CREATE_APM()
    auto resp = get<model::Catalog, std::string_view>( req.req, "identifier"sv, args["identifier"sv], methods, apm, &userRole );
    FINISH_APM()
  }, "./paths/catalog.yaml#/identifier"sv );

  server.addHandler( "GET"sv, "/catalog/customer/code/{code}"sv, []( const RoutingRequest& req, auto args )
  {
    CREATE_APM()
    auto resp = retrieveAll<model::Catalog>( req.req, args["code"sv], methods, apm );
    FINISH_APM()
  }, "./paths/catalog.yaml#/customer"sv );

  server.addHandler( "GET"sv, "/catalog/count/references/{id}"sv, []( const RoutingRequest& req, auto&& args )
  {
    CREATE_APM()
    auto resp = refcounts<model::Catalog>( req.req, args["id"sv], methods, apm, &userRole );
    FINISH_APM()
  }, "./paths/catalog.yaml#/refcount"sv );

  server.addHandler( "GET"sv, "/catalog/history/summary/{id}"sv, []( const RoutingRequest& req, auto&& args )
  {
    CREATE_APM()
    auto resp = versionHistorySummary<model::Catalog>( req.req, args["id"sv], methods, apm, &superuserRole );
    FINISH_APM()
  }, "./paths/catalog.yaml#/historySummary"sv );

  server.addHandler( "GET"sv, "/catalog/history/document/{id}"sv, []( const RoutingRequest& req, auto&& args )
  {
    CREATE_APM()
    auto resp = versionHistoryDocument<model::Catalog>( req.req, args["id"sv], methods, apm, &superuserRole );
    FINISH_APM()
  }, "./paths/catalog.yaml#/historyDocument"sv );

  server.addHandler( "GET"sv, "/catalog/{type}/between/{start}/{end}"sv, []( const RoutingRequest& req, auto&& args )
  {
    CREATE_APM()
    auto resp = between<model::Catalog>( req, std::forward<boost::container::flat_map<std::string_view, std::string_view>>( args ), methods, apm, &userRole );
    FINISH_APM()
  }, "./paths/catalog.yaml#/between"sv );

  server.addHandler( "POST"sv, "/catalog/"sv, []( const RoutingRequest& req, auto&& )
  {
    CREATE_APM()
    auto resp = create<model::Catalog>( req.req, req.body, methods, apm, &adminRole );
    FINISH_APM()
  }, "./paths/catalog.yaml#/root"sv );

  server.addHandler( "PUT"sv, "/catalog/id/{id}"sv, []( const RoutingRequest& req, auto&& args )
  {
    CREATE_APM()
    auto resp = update<model::Catalog>( req.req, req.body, args["id"sv], methods, apm, &adminRole );
    FINISH_APM()
  }, "./paths/catalog.yaml#/update"sv );

  server.addHandler( "DELETE"sv, "/catalog/id/{id}"sv, []( const RoutingRequest& req, auto args )
  {
    CREATE_APM()
    auto resp = remove<model::Catalog>( req.req, args["id"sv], methods, apm, &adminRole );
    FINISH_APM()
  }, "./paths/catalog.yaml#/id"sv );
}

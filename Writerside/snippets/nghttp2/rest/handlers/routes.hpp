//
// Created by Rakesh on 13/02/2021.
//

#include "http/common.hpp"
#include "http/template.hpp"

void spt::http2::rest::handlers::addRoutes( http2::framework::Router<Response>& router )
{
  static const std::array methods{ "DELETE"s, "GET"s, "OPTIONS"s, "POST"s, "PUT"s };

  router.add( "GET"sv, "/entity/user/"sv, []( const http2::framework::RoutingRequest& req, auto&& )
  {
    return retrieveAll<model::User>( req.req, methods, &userRole );
  }, "./paths/user.yaml#/root"sv );

  router.add( "GET"sv, "/entity/user/id/{id}"sv, []( const http2::framework::RoutingRequest& req, auto args )
  {
    return get<model::User>( req.req, args["id"sv], methods, &userRole );
  }, "./paths/user.yaml#/id"sv );

  router.add( "GET"sv, "/entity/user/identifier/{identifier}"sv, []( const http2::framework::RoutingRequest& req, auto args )
  {
    return get<model::User, std::string_view>( req.req, "identifier"sv, args["identifier"sv], methods, &userRole );
  }, "./paths/user.yaml#/identifier"sv );

 router.add( "GET"sv, "/entity/user/count/references/{id}"sv, []( const http2::framework::RoutingRequest& req, auto args )
  {
    return refcounts<model::User>( req.req, args["id"sv], methods, &userRole );
  }, "./paths/user.yaml#/refcount"sv );

  router.add( "GET"sv, "/entity/user/history/summary/{id}"sv, []( const http2::framework::RoutingRequest& req, auto args )
  {
    return versionHistorySummary<model::User>( req.req, args["id"sv], methods, &superuserRole );
  }, "./paths/user.yaml#/historySummary"sv );

  router.add( "GET"sv, "/entity/user/history/document/{id}"sv, []( const http2::framework::RoutingRequest& req, auto args )
  {
    return versionHistoryDocument<model::User>( req.req, args["id"sv], methods, &superuserRole );
  }, "./paths/user.yaml#/historyDocument"sv );

  router.add( "GET"sv, "/entity/user/{type}/between/{start}/{end}"sv, []( const http2::framework::RoutingRequest& req, auto args )
  {
    auto type = args["type"sv];
    if ( type != "created"sv && type != "modified"sv ) return error( 404, methods, req.req.header );
    auto prop = std::format( "metadata.{}"sv, type );

    auto svar = spt::util::parseISO8601( args["start"sv] );
    if ( !svar.has_value() )
    {
      LOG_WARN << "Invalid start date in path " << req.req.path;
      return error( 400, svar.error(), methods, req.req.header );
    }
    auto evar = spt::util::parseISO8601( args["end"sv] );
    if ( !evar.has_value() )
    {
      LOG_WARN << "Invalid end date in path " << req.req.path;
      return error( 400, evar.error(), methods, req.req.header );
    }

    return between<model::User>( req.req, prop, svar.value(), evar.value(), methods, &userRole );
  }, "./paths/user.yaml#/between"sv );

  router.add( "POST"sv, "/entity/user/"sv, []( const http2::framework::RoutingRequest& req, auto&& )
  {
    return create<model::User>( req.req, req.body, methods, &adminRole );
  }, "./paths/user.yaml#/root"sv );

  router.add( "PUT"sv, "/entity/user/id/{id}"sv, []( const http2::framework::RoutingRequest& req, auto args )
  {
    return update<model::User>( req.req, req.body, args["id"sv], methods, &adminRole );
  } );

  router.add( "DELETE"sv, "/entity/user/id/{id}"sv, []( const http2::framework::RoutingRequest& req, auto args )
  {
    return remove<model::User>( req.req, args["id"sv], methods, &adminRole );
  } );
}
//
// Created by Rakesh on 10/10/2020.
//

#pragma once

#include "model/entitiesquery.hpp"
#include "model/jwttoken.hpp"

#include <sstream>
#include <tuple>
#include <vector>

#include <http2/framework/request.hpp>
#include <http2/framework/response.hpp>

namespace spt::http2::rest::handlers
{
  framework::Response unsupported( const std::vector<std::string>& methods, const nghttp2::asio_http2::header_map& headers );
  framework::Response error( uint16_t code, const std::vector<std::string>& methods, const nghttp2::asio_http2::header_map& headers );
  framework::Response error( uint16_t code, std::string_view message, const std::vector<std::string>& methods, const nghttp2::asio_http2::header_map& headers );

  void cors( const nghttp2::asio_http2::header_map& headers, const nghttp2::asio_http2::server::response& res,
      const std::string& methods = {"DELETE,GET,OPTIONS,PATCH,POST,PUT"} );

  using AuthResponse = std::tuple<uint16_t, model::JwtToken::Ptr>;
  AuthResponse authorise( const Request& req );
  std::string correlationId( const Request& req );

  bool hasEntityAccess( std::string_view entity, const model::JwtToken& token );
  bool superuserRole( std::string_view entity, const model::JwtToken& token );
  bool adminRole( std::string_view entity, const model::JwtToken& token );
  bool userRole( std::string_view entity, const model::JwtToken& token );

  std::string statusMessage( int16_t status );

  using EntitiesQueryResponse = std::tuple<uint16_t, std::optional<model::EntitiesQuery>>;
  EntitiesQueryResponse parseQuery( const Request& req, const std::vector<std::string_view>& additional = {} );

  using Output = std::tuple<std::string,bool>;
  Output compress( std::stringstream& oss );

  bool shouldCompress( const Request& req );
}
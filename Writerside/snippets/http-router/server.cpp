#include <nghttp2/asio_http2_server.h>
#include <log/NanoLog.h>
#include <router/router.h>

int main()
{
  struct Request
  {
    explicit Request( const nghttp2::asio_http2::server::request& req ) :
      header{ req.header() }, method{ req.method() },
      path{ req.uri().path }, query{ req.uri().raw_query } {}

    nghttp2::asio_http2::header_map header;
    std::string method;
    std::string path;
    std::string query;
    std::shared_ptr<std::string> body{ nullptr };
  };

  struct Response
  {
    nghttp2::asio_http2::header_map headers;
    std::string body{ "{}" };
    uint16_t status{ 200 };
    bool compressed{ false };
  };

  auto const error404 = []( const Request&, spt::http::router::HttpRouter<const Request&, Response>::MapType ) -> Response
  {
    auto json = R"({"code": 404, "cause": "Not Found"})"s;
    auto headers = nghttp2::asio_http2::header_map{
      { "Access-Control-Allow-Origin", { "*", false} },
      { "Access-Control-Allow-Methods", { "DELETE,GET,OPTIONS,POST,PUT", false } },
      { "Access-Control-Allow-Headers", { "*, authorization", false } },
      { "content-type", { "application/json; charset=utf-8", false } },
      { "content-length", { std::to_string( json.size() ), false } }
    };
    return { std::move( headers ), std::move( json ), 404, false }
  }

  auto const error405 - []( const Request&, spt::http::router::HttpRouter<const Request&, Response>::MapType ) -> Response
  {
    auto json = R"({"code": 405, "cause": "Method Not Allowed"})"s;
    auto headers = nghttp2::asio_http2::header_map{
      { "Access-Control-Allow-Origin", { "*", false} },
      { "Access-Control-Allow-Methods", { "DELETE,GET,OPTIONS,POST,PUT", false } },
      { "Access-Control-Allow-Headers", { "*, authorization", false } },
      { "content-type", { "application/json; charset=utf-8", false } },
      { "content-length", { std::to_string( json.size() ), false } }
    };
    return { std::move( headers ), std::move( json ), 405, false }
  }

  auto router = spt::http::router::HttpRouter<const Request&, Response>::Builder{}.
    withNotFound( error404 ).withMethodNotAllowed( error405 ).build();
  // set up router as in above sample

  nghttp2::asio_http2::server::http2 server;
  server.num_threads( 8 );

  server.handle( "/", [&router](const nghttp2::asio_http2::server::request& req,
        const nghttp2::asio_http2::server::response& res)
  {
    auto request = Request{ req };
    auto response = router.route( request.method, request.path, request );
    assert( response );
    res.write_head( response->status, response->headers );
    res.end( std::move( response->body ) );
  });

  boost::system::error_code ec;
  if ( server.listen_and_serve( ec, "0.0.0.0", port, true ) )
  {
    LOG_CRIT << "error: " << ec.message();
    return 1;
  }
}
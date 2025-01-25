// metrics.cpp
#include "catch.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http/field.hpp>

namespace
{
  namespace spt::integration
  {
    void metrics(const std::string& token = {})
    {
      namespace beast = boost::beast;     // from <boost/beast.hpp>
      namespace http = beast::http;       // from <boost/beast/http.hpp>
      namespace net = boost::asio;        // from <boost/asio.hpp>
      using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

      net::io_context ioc;
      tcp::resolver resolver(ioc);
      beast::tcp_stream stream(ioc);

      auto const results = resolver.resolve("localhost", "9000");
      stream.connect(results);

      http::request<http::string_body> req{http::verb::get, "/metrics", 11};
      req.set(http::field::host, "localhost");
      req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
      if (!token.empty()) req.set("Authorization", "Bearer " + token);
      http::write(stream, req);

      beast::flat_buffer buffer;
      http::response<http::string_body> res;
      http::read(stream, buffer, res);

      REQUIRE(res.body().size() > 0);

      const auto ct = res[http::field::content_type];
      REQUIRE_FALSE(ct.empty());
      REQUIRE(ct == "text/plain");

      beast::error_code ec;
      stream.socket().shutdown(tcp::socket::shutdown_both, ec);
      if (ec && ec != beast::errc::not_connected) throw beast::system_error{ec};
    }
  }
}

SCENARIO( "Service metrics endpoint", "[status]" )
{
  GIVEN("IoT receiver service is running")
  {
    THEN("Can access metrics endpoint without authentication")
    {
      REQUIRE_NOTHROW(spt::integration::metrics());
    }

    AND_THEN("Can access metrics endpoint with authentication")
    {
      REQUIRE_NOTHROW(spt::integration::metrics("<complex valid token>"));
    }

    AND_THEN("Can access metrics endpoint with invalid authentication")
    {
      REQUIRE_NOTHROW(spt::integration::metrics("abc123"));
    }
  }
}
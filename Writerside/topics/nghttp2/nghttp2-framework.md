# Framework
A simple framework for configuring and running a `nghttp2-asio` service.  Uses the
more powerful [router](https://github.com/sptrakesh/http-router) and delegates all
registered handlers to run on a separate worker thread pool.  This prevents client
requests from blocking the main server handling event loop.

<tabs id="nghttp2-asio-framework">
  <tab title="Response" id="nghttp2-asio-framework-response">
    A <i>concept</i> that defines the minimum expected interface for a user defined <strong>Response</strong> structure.
    <code-block lang="C++" src="nghttp2/asio/response.hpp" collapsible="true"/>
  </tab>
  <tab title="Request" id="nghttp2-asio-framework-request">
    A simple wrapper around a <code>nghttp2::asio_http2::server::request</code> object.  Used to ensure that important data such as the request <code>method, path, query</code> and <code>headers</code> are available in the worker threads even if the client closes the connection while the request is being processed.
    <code-block lang="C++" src="nghttp2/asio/request.hpp" collapsible="true"/>
  </tab>
  <tab title="Configuration" id="nghttp2-asio-framework-configuration">
    Simple structure used to configure the server.  Most fields have a default value, but the <code>origins</code> field <strong>must</strong> be set with the origins the server will handle.
    <code-block lang="C++" src="nghttp2/asio/configuration.hpp" collapsible="true"/>
  </tab>
  <tab title="Server" id="nghttp2-asio-framework-server">
    A <strong>server</strong> instance which can be configured with the desired endpoint handler functions to respond to client requests.  The framework assumes the server will handle only text input (JSON, YAML, ...) as is common with API implementations.  The user defined <code>Response</code> object conforming to the <code>Response concept</code> should be specified as the template type for the server instance.
    <code-block lang="C++" src="nghttp2/asio/server.hpp" collapsible="true"/>
  </tab>
  <tab title="Response Example" id="nghttp2-asio-framework-response-sample">
    <code-block lang="C++" src="nghttp2/asio/response.cpp" collapsible="true"/>
  </tab>
</tabs>

## Usage
The standard workflow for creating a `server` instance is as follows:
* Define the desired `Response` structure that conforms to the `spt::http2::framework::Response` *concept*.
  * The structure should be constructible from a `nghttp2::asio_http2::header_map`.  Note that the headers
    specified are the client request headers, and not the response headers.  The CTOR may use the client
    headers as appropriate for generating the response.
  * Implement the `set` method that accepts as input `std::span<std::string>` instances that define the HTTP 
    methods/verbs the endpoint supports, and the *origins* the server serves.
* Create a `spt::http2::framework::Configuration` instance.
  * Set the `origins` field as appropriate.
  * Change other fields as appropriate.
* Create a `server` instance with the `Response` type and the `spt::http2::framework::Configuration` instance.
  * Register the desired endpoint handler functions. 
  * Start the server.
* Block the main thread until a stop/kill signal is received.
* Stop the server when a signal is received.

### Example
<code-block lang="C++" collapsible="true">
<![CDATA[
#include <http2/framework/server.hpp>
  ...
  auto conf = spt::http2::framework::Configuration{};
  conf.port = 8080;
  conf.origins = std::vector{ "https://dashboard.sptci.com", "https://admin.sptci.com", "https://app.sptci.com" };
  auto server = spt::http2::framework::Server<Response>{ conf };

  // Add handlers
  server.addHandler( "GET", "/some/path/{param}", []( const spt::http2::framework::RoutingRequest& rr, const auto& params )
  {
    auto resp = Response( rr.req.header );
    resp.headers.emplace( "content-type", "application/json" );
    resp.body = boost::json::serialize( boost::json::object{
      { "code", 200 },
      { "cause", "ok" },
      { "parameter", params.at( "param" ) }
    } );
    return resp;
  } );

  // Add other handlers as appropriate
  server.start();

  // Run until signal to stop is received
  auto ioc = boost::asio::io_context{};
  boost::asio::signal_set signals( ioc, SIGINT, SIGTERM );
  signals.async_wait( [&server](auto const&, int) { server.stop(); } );
  ioc.run();
]]>
</code-block>

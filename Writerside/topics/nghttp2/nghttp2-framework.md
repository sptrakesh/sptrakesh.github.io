# HTTP2 Framework
A simple [framework](https://github.com/sptrakesh/nghttp2-asio/tree/framework)
for configuring and running a `nghttp2-asio` service.  Uses the
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
    Simple `Response` structure that conforms to the `concept` as used in the test suite.
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

### Additional customisation
A couple of additional extension features are available.

#### Scanner
A `callback` function can be registered when creating the **Server** instance.  This function
will be called with the raw data submitted by clients as payload.  The function can scan the
data and return `false` if it detects any harmful content (tags, scripts, sql, ...).  The
callback has signature `std::function<bool( std::string_view )>`.

#### Extra Response Processing
Additional logic can be injected into the response processing workflow by specialising the
`void extraProcess<Response>( const Request& req, Response& resp, boost::asio::thread_pool& pool )`
function.  This can be used to execute additional logic before *committing* the response.
Use to publish metrics, custom logs etc. related to the request/response cycle.

<code-block lang="C++" collapsible="true">
<![CDATA[
namespace spt::http2::framework
{
  template <>
  void extraProcess( const Request& req, ptest::Response& resp, boost::asio::thread_pool& pool )
  {
    auto str = std::format( "Extra processing for {} to {} using {}.", req.method, req.path, typeid( resp ).name() );
    boost::asio::post( pool, [str]
    {
      ++ptest::counter;
      LOG_INFO << str;
    } );
  }
}
]]>
</code-block>

### Header Only
The library can be used *header only* is so desired.  Implement the `statusMessage` and
`cors` functions are desired.  The [s3-proxy](s3-proxy.md) server uses this model and does
not link to the framework library.

```C++
std::string_view spt::http2::framework::statusMessage( uint16_t status );
void spt::http2::framework::cors( const nghttp2::asio_http2::server::request& req, const nghttp2::asio_http2::server::response& res, const Configuration& configuration );
```

### Example
Simple example server to illustrate full use of the framework.  The attached `CMakeLists.txt`
file manually links to the framework and `nghttp2_asio`, since I have been unsuccessful
in getting the configuration for the `nghttp2_asio` library right.

<tabs id="http2-framework-example">
  <tab title="cmake" id="http2-framework-example-cmake">
    <code-block lang="CMake" src="nghttp2/asio/CMakeLists.txt" collapsible="true"/>
  </tab>
  <tab title="server" id="http2-framework-example-server">
    <code-block lang="C++" src="nghttp2/asio/server.cpp" collapsible="true"/>
  </tab>
</tabs>

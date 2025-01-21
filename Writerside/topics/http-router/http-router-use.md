# HTTP Router Use
The **HttpRouter<Request, Response, Map>** class exposes two primary methods -
`add` and `route` - that are used to set up and perform routing:
* **CTOR** - Create an instance with the optional handlers to handle standard
  scenarios such as *Not Found (404)*, *Method Not Allowed (405)*, and
  *Internal Server Error (500)*.
  * Use the **Builder** to specify the desired error handlers and initialise the
    router in a more convenient manner.
* **add** - Use to add paths or parametrised paths to the router.
  * This is thread safe.  Configuring routing should generally not need
    thread safety, but just in case route additions are set up in parallel in a
    multi-threaded environment, a `std::mutex` is used to ensure thread safety.
    * The general expectation (standard pattern when configuring routes) is
      that all the routes are configured before the server starts routing requests.
    * Performing routing while additional routes are being added to the router
      could lead to undefined behaviour.
  * Duplicate routes will throw a [`spt::http::router::DuplicateRouteError`](https://github.com/sptrakesh/http-router/blob/master/src/error.hpp)
    exception.
  * Routes with invalid parameter will throw a [`spt::http::router::InvalidParameterError`](https://github.com/sptrakesh/http-router/blob/master/src/error.hpp) exception.
    * This is thrown if a parameter uses the `:<parameter>` form.
    * This is thrown if a parameter does not end with the `}` character.
* **route** - When a client request is received, delegate to the router to handle
  the request.
  * If a *notFound* handler was specified when creating the router (first optional
    constructor parameter), and the input request *path* was not found, the
    handler will be invoked.
  * If a *methodNotFound* handler was specified when creating the router (second
    optional constructor parameter), and the input request *method* was not
    configured for the specified *path*, the handler will be invoked.
  * If a *errorHandler* handler was specified when creating the router (third
    optional constructor parameter), and an exception was thrown by the configured
    handler function for the *method:path*, the handler will be invoked.
* If Boost has been found a few additional utility methods are exposed.
  * **json** - Output the configured routes and some additional metadata as a
    JSON structure.  See the sample output below from the [device](https://github.com/sptrakesh/http-router/blob/master/test/device.cpp) test.
  * **str** - Output the configured routes and some additional metadata as a string.
    This is just the JSON representation serialised.
  * **operator<<** - Appends the string representation to the output stream.
* **yaml** - Output the configured routes in YAML format which can be embedded
  or cross-verified against the API OpenAPI Specifications file.  If using this
  feature, please try to specify the optional `ref` parameter to the *add* method.
  For example see the output below from the [device](https://github.com/sptrakesh/http-router/blob/master/test/device.cpp) test.

## Sample Output

### JSON
<code-block lang="JSON" src="http-router/routes.json" collapsible="true"/>

### YAML
<code-block lang="YAML" src="http-router/routes.yaml" collapsible="true"/>

## Example
The following shows sample use of the router.  See [basic](https://github.com/sptrakesh/http-router/blob/master/test/basic.cpp) test
and other unit tests for more samples.

<code-block lang="c++" src="http-router/example.cpp" collapsible="true"/>

### Use with nghttp2
The `route` method returns a `std::optional<Response>`.  If no configured path
matches, returns `std::nullopt` (or the response from the not found handler if
specified at construction time).  Otherwise, returns the response from the
callback function.

<code-block lang="c++" src="http-router/server.cpp" collapsible="true"/>

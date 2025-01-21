# HTTP Router
Simple general purpose HTTP path based request router.  Requires a compiler with
C++20 support.  No assumption is made on the type of framework being used.
We have used it mainly with [nghttp2](nghttp2-asio.md)
* Supports static and parametrised URI paths.
* Parameters (slugs) are represented using curly brace enclosed name `{param}`.
  * Curly brace form was chosen in favour of `:param` for sorting purpose.
  * Sorting with `{` implies use of non-ascii characters in path will have inconsistent behaviour.
* Wildcard path pattern is supported.
  * Only a single wildcard (`*`) as the last *component* of the URI path is supported.
  * Parameters (slugs) are supported in the same URI path (`/device/sensor/id/{id}/*`).
  * Internally the wildcard character `*` is replaced by `~` for sorting purpose.
  * As a consequence of sorting, it is possible to configure more specific URI
    wildcard paths in combination with wildcard paths at the base level. Example:
    `/device/sensor/*` and `/device/sensor/id/*`.
* Templated on the **Response** type and an input **Request**.  Optionally
  specify the type of *Map* container to use to hold the parsed path parameters.
  Defaults to `boost::container::flat_map` if [boost](https://boost.org/) is found,
  or to `std::map`.  The type specified must be interface compatible with
  `std::map`.  The `key` and `value` must be either `std::string_view` or `std::string`.
* Function based routing.  Successful matches are *routed* to the specified
  *handler* callback function.
  * Parameters are returned as a *map*.  The type of map is determined via the
    optional third template parameter.
  * Callback function has signature `Response( Request, MapType<String, String>&& )`
    where `MapType` is either `boost::container::flat_map` or `std::map` (if
    using defaults, or the container you specify) and
    `String` is `std::string_view` (if using defaults) or `std::string` if you
    specify.  See [string.cpp](https://github.com/sptrakesh/http-router/blob/master/test/string.cpp) test for sample of specifying
    your preferred container and `std::string` as the type in the container.
  * The `MapType` will hold the parsed *parameter->value* pairs.
  * The path part matching the wildcard (for wildcard paths) is added to the
    `MapType` as `_wildcard_` key.  Keep this in mind when naming path parameters
    for wildcard paths.

## Install
No install is necessary.  Copy the [router.hpp](https://github.com/sptrakesh/http-router/blob/master/src/router.hpp),
[split.hpp](https://github.com/sptrakesh/http-router/blob/master/src/split.hpp),
and [concat.hpp](https://github.com/sptrakesh/http-router/blob/master/src/concat.hpp)
files into your project and use.

The headers may be installed into a standard location using `cmake`.

```shell
git clone https://github.com/sptrakesh/http-router.git
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local/spt -S . -B build
sudo cmake --install build
```
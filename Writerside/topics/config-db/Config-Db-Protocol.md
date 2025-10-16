# Protocol
<show-structure for="chapter,tab"/>

Service supports both TCP/IP and HTTP/2 connections. The TCP service uses 
[flatbuffers](https://google.github.io/flatbuffers/) as the data interchange format, while 
the HTTP/2 service uses JSON.

The models were generated from the schema files using the following command:

```shell
(cd <path to project>/src/common/model;
<path to>/flatc --cpp --cpp-std c++17 --cpp-static-reflection --reflect-names request.fbs response.fbs tree.fbs)
```

## TCP/IP

The TCP/IP service uses flatbuffers for data interchange. The wire protocol adds an extra `4` bytes
at the beginning of each message. These `4` bytes contain the size of the message being 
interchanged. Client and server will use these `4` bytes to ensure they have read the 
entire message before attempting to marshall the data.

All requests are for setting or retrieving multiple *keys*. When setting or deleting 
multiple keys, a single failure will result in rolling back the entire *transaction*.

## HTTP/2

The HTTP service uses JSON for data interchange. HTTP/1.1 or upgrade to 2 is not supported 
directly. Clients wishing regular HTTP/1.1 or upgrade feature can use 
[envoy](https://www.envoyproxy.io/) to handle these as well as *https*.

The HTTP service always responds with a JSON message. Input message is only supported on 
the `PUT` endpoints, and only accept a plain text body which is the desired *value* to set 
for the *key*.

The following path prefixes are supported by the service:
* URI paths with the `/key` *prefix*. The rest of the path is interpreted as the desired 
  `key` within the configuration database. This is used to manage the configuration data.
* URI paths with the `/list` *prefix*. The rest of the path is interpreted as the desired 
  `key` within the configuration database. This is used to retrieve child node names for
  the specified *path*.

See [shell](https://github.com/sptrakesh/config-db/blob/master/test/integration/curl.sh) 
and/or [cpr](https://github.com/sptrakesh/config-db/blob/master/test/integration/http.cpp)
for sample HTTP requests and responses.

### Custom headers

Request options as defined in the flatbuffers schema can be specified using custom HTTP headers when making request to the HTTP/2 service.

* `x-config-db-if-not-exists` - Use to specify the `if_not_exists` property. A value of 
  `true` is interpreted as the boolean value `true`. Any other values are interpreted as 
  `false`. See the `PutNotExists` function in the example.
* `x-config-db-ttl` - Use to specify the `expiration_in_seconds` property. Must be a numeric 
  value, and represent the expiration from current time in *seconds*.
* `x-config-db-cache` - Use to specify the `cache` property. A value of `true` is 
  interpreted as the boolean value `true`. Any other values are interpreted as `false`.

**Note:** The HTTP service does not support batch (multiple key) operations.

**Note:** The HTTP service does not support move operations.

### PUT
```shell
# PUT request
url='http://localhost:6006/key/test'
curl -s --http2-prior-knowledge -XPUT -H "content-type: text/plain" -d "value" $url
```
```shell
{"code": 200, "cause": "Ok"}
```

### PutNotExists
```shell
# PUT request
url='http://localhost:6006/key/test'
curl -s --http2-prior-knowledge -XPUT -H "content-type: text/plain" -H "x-config-db-if-not-exists: true" -d "value" $url
```
```shell
{"code": 412, "cause": "Unable to save"}
```

### PutWithTTL
```shell
curl -s --http2-prior-knowledge -XPUT \
  -H "content-type: text/plain" \
  -H "x-config-db-if-not-exists: true" \
  -H "x-config-db-ttl: 5" \
  -d "value" 'http://localhost:6006/key/test'
```
```shell
{"code": 200, "cause": "Ok"}
```

### GET
```shell
# GET request
url='http://localhost:6006/key/test'
curl -s --http2-prior-knowledge $url
```
```shell
{"key":"/test","value":"value"}
```

### DELETE
```shell
# DELETE request
url='http://localhost:6006/key/test'
curl -s --http2-prior-knowledge -XDELETE $url
```
```shell
{"code": 200, "cause": "Ok"}
```

### GET after delete
```shell
# GET request after delete
url='http://localhost:6006/key/test'
curl -s --http2-prior-knowledge $url
```
```shell
{"code": 404, "cause": "Not found"}
```

## Messages
The following messages are transferred between the client and TCP server.

### Request

The [request](https://github.com/sptrakesh/config-db/blob/master/src/common/model/request.fbs)
message contains the `action` desired as well as the `key-value` pairs to be sent to the 
service. The `value` may be omitted for all actions other than `Put`.

* **action** - An `enum` used to specify the type of action to perform against the service.
  Action values are kept similar to their HTTP verb counterparts when possible.
* **data** - Array of `key-value` pairs.
  * **key** - The `key` to act upon against the service.
  * **value** - The `value` to set for the `key`. This is only relevant for the `Put` 
    or `Move` actions. In the case of `Move`, the `value` is the destination `key` to move
    the `value` to.
  * **options** - Additional options that relate to setting (or moving) *keys*.
    * **if_not_exists** - Specify `true` if `key` should be saved *only* if it does not 
      exist in the database. For `Move`, this applies to the `value` which serves as the 
      destination `key`. 
    * **expiration_in_seconds** - TTL in seconds from request time. The `key` will be 
      automatically deleted by the system after the specified time has elapsed.
    * **cache** - Indicate that the `key-value` pair is to be treated as a *cache* entry.
      Cached keys are maintained in the *tree* structure, and are meant to be used purely 
      as temporary values that can be looked up by *unique* `key`. Cached keys must have a TTL.

### Response

The [response](https://github.com/sptrakesh/config-db/blob/master/src/common/model/response.fbs)
message contains either the `value` for the array of *keys* (`Get`, `List`), or a `boolean`
indicating success or failure of the *transaction* (`Put`, `Delete`).

A **KeyValueResult** structure is used to represent the result for a specific `key` sent 
in the request. The `value` can be one of:
* **Value** - The string value associated with the specified `key` for a `Get` request.
* **Children** - A list of child node names for the specified *key/path* for a `List` request.
* **Success** - A *boolean* value used to report failure at retrieving the `key` or `path`.

When reading the response, as mentioned above, the first `4` bytes represent the length 
of the buffer being returned, and the rest of the bytes (use loops and similar constructs
to read until the full message has been received) will be the buffer. See 
[integration test](https://github.com/sptrakesh/config-db/blob/master/test/integration/tcp.cpp)
for an example.

### Ping

Short (less than 5 bytes) messages may be sent to the service as a *keep-alive* message. 
Service will echo the message back to the client. Examples include `ping`, `noop`, etc.

## API

A high-level client API to interact with the service is provided. The interface hides the 
complexities involved with making TCP/IP requests using flatbuffers. 

<tabs id="configdb-api">
  <tab title="C++" id="configdb-api-cpp">
The <a href="https://github.com/sptrakesh/config-db/blob/master/src/api/api.hpp">api</a> presents an 
interface that is very similar to the persistence interface used internally by the
service. The API maintains a connection pool to the service and performs the 
required interactions using the flatbuffer models.

**Note:** API must be initialised via the `init` function before first use.

See <a href="https://github.com/sptrakesh/config-db/blob/master/test/integration/apicrud.cpp">integration test</a>
for sample usage of the API. The *shell* application is built using the client API. See
<a href="https://github.com/sptrakesh/config-db?tab=readme-ov-file#api-usage">cmake</a> for including
the API in your cmake project.

<code-block lang="c++" collapsible="false">
<![CDATA[
#include <log/NanoLog.hpp>
#include <configdb/api/api.hpp>
...
  using namespace spt::configdb::api;
  nanolog::set_log_level( nanolog::LogLevel::DEBUG );
  nanolog::initialize( nanolog::GuaranteedLogger(), "/tmp/", "config-db-itest", false );
  init( "localhost", "2022", false );

  const auto status = set( "key"sv, "value"sv );
  assert( status );

  const auto value = get( key );
  assert( value );
  assert( *value == "value"sv );

  const auto dstatus = remove( "key"sv );
  assert( dstatus );
]]>
</code-block>
  </tab>
  <tab title="Rust" id="configdb-api-rust">
A simple wrapper around the C++ API using <a href="https://cxx.rs/">cxx.rs</a>.
See <a href="https://github.com/sptrakesh/config-db/tree/master/client/rust">documentation</a>
for details.

<code-block lang="Rust" collapsible="false">
<![CDATA[
use configdb::*;

#[test]
fn operations()
{
  let mut logger = Logger::new("/tmp/", "configdb-rust");
  logger.level = LogLevel::DEBUG;
  init_logger(logger);

  let mut conf = Configuration::new("localhost", 2022);
  conf.ssl = false;
  init(conf);

  let mut rd = RequestData::new("key", "value");
  rd.expirationInSeconds = 60;
  let result = set(&rd);
  assert_eq!(result, true);

  let result = get(rd.key.as_str());
  assert!(result.is_ok());
  assert_eq!(result.unwrap(), rd.value);

  let result = ttl(format!("/{}", rd.key).as_str());
  assert!(result > 0);

  let result = remove(rd.key.as_str());
  assert_eq!(result, true);
}
]]>
</code-block>
  </tab>
  <tab title="Python" id="configdb-api-python">
See <a href="https://github.com/sptrakesh/config-db/tree/master/client/python">documentation</a>
for details.

<code-block lang="Python" collapsible="false">
<![CDATA[
async with Client(host="localhost", port=2020) as client:
    _key = "/key1/key2/key3"
    res = await client.set(_key, "value")
    res = await client.get(_key)
    log.info(f"Read stored value: {res}")
]]>
</code-block>
  </tab>
</tabs>
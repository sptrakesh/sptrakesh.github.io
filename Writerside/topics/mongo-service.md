# Mongo Service

A service that is primarily used to automatically create *versions* of each
document created/updated/deleted in a MongoDB instance.  The service also
acts as a proxy for accessing the database.  This reduces the number of
connections to the database, and also reduces the need to share credentials
with upstream processes/applications.  The service itself does **not** support
any kind of *authentication/authorisation*, and hence is meant to be used only
in *private networks* and accessed by processes that run within the **same network**.

## Version History
All documents stored in the database will automatically be *versioned* on save.
Deleting a document will move the current document into the *version history*
database *collection*.  This makes it possible to retrieve previous *versions*
of a document as needed, as well as restore a document (regardless of whether
it has been *deleted* or not).

## Protocol
All interactions are via *BSON* documents sent to the service.  Each request must
conform to the following document model:
* `action (string)` - The type of database action being performed.  One of
  `create|retrieve|update|delete|count|distinct|index|dropCollection|dropIndex|bulk|pipeline|transaction|createTimeseries|createCollection|renameCollection`.
* `database (string)` - The Mongo database the action is to be performed against.
  - Not needed for `transaction` action.
* `collection (string)` - The Mongo collection the action is to be performed against.
  - Not needed for `transaction` action.
* `document (document)` - The document payload to associate with the database operation.
  For `create` and `update` this is assumed to be the document that is being saved.
  For `retrieve` or `count` this is the *query* to execute.  For `delete` this
  is can be a simple `document` with an `_id` field, or a filter to identify the
  documents to delete.
* `options (document)` - The options to associate with the Mongo request.  These correspond
  to the appropriate options as used by the Mongo driver.
* `metadata (document)` - Optional *metadata* to attach to the version history document that
  is created (not relevant for `retrieve` obviously).  This typically will include
  information about the user performing the action, any other information as
  relevant in the system that uses this service.
* `application` - Optional name of the *application* accessing the service.
  Helps to retrieve database metrics for a specific *application*.
* `correlationId (string)` - Optional *correlation id* to associate with the metric record
  created by this action.  This is useful for tracing logs originating from a single
  operation/request within the overall system.  This value is stored as a *string*
  value in the *metric* document to allow for sensible data types to used as the
  *correlation id*.
* `skipVersion (bool)` - Optional `bool` value to indicate not to create a *version history*
  document for this `action`.  Useful when creating non-critical data such as
  logs.
* `skipMetric (bool)` - Optional `bool` value to indicate not to create a *metric*
  document for this `action`.  Useful when calls are made a part of a monitoring framework, and volume of metrics
  generated overwhelms storage requirements

## Serialisation
A simple serialisation framework is also provided.  Uses the
[visit_struct](https://github.com/cbeck88/visit_struct) library to automatically serialise and deserialise
*visitable* classes/structs.

### BSON
A simple serialisation framework to serialise and deserialise *visitable* classes/structs to and from BSON.

The framework provides the following primary functions to handle (de)serialisation:
* `marshall<Model>( const Type& )` - to marshall the specified object to a BSON document.
* `unmarshall<Model>( bsoncxx::document::view view )` - unmarshall the BSON document into a default constructed object.
* `unmarshall<Model>( Model& m, bsoncxx::document::view view )` - unmarshall the BSON document into the specified model instance.

The framework handles non-visitable members within a visitable root object.  Custom implementations can be implemented.
* For non-visitable classes/structs, implement the following functions as appropriate:
  * `bsoncxx::types::bson_value::value bson( const <Class/Struct Type>& model )` that will produce a BSON document as a value variant for the data encapsulated in the object.
  * `void set( <Class/Struct Type>& field, bsoncxx::types::bson_value::view value )` that will populate the model instance from the BSON value variant.
* For partially visitable classes/structs, implement the following `populate` callback functions as appropriate:
  * `void populate( const <Class/Struct Type>& model, bsoncxx::builder::stream::document& doc )` to add the non-visitable fields to the BSON stream builder.
  * `void populate( <Class/Struct Type>& model, bsoncxx::document::view view )` to populate the non-visitable fields in the object from the BSON document.

### JSON
A simple serialisation framework to serialise and deserialise *visitable* classes/structs
to and from JSON.

Similar to the BSON framework, the JSON framework also handles non-visitable members within a visitable root object.
Custom implementations can be implemented.
* For non-visitable classes/structs, implement the following functions as appropriate:
  * `boost::json::value json( const <Class/Struct Type>& model )` that will produce a JSON value for the data encapsulated in the object.
  * `void set( const char* name, <Class/Struct Type>& field, simdjson::ondemand::value& value )` that will populate the model instance from the JSON value.
* For partially visitable classes/structs, implement the following `populate` callback functions as appropriate:
  * `void populate( const <Class/Struct Type>& model, boost::json::object& object )` to add non-visitable fields to the JSON object.
  * `void populate( const <Class/Struct Type>& model, simdjson::ondemand::object& object )` to populate non-visitable fields from the JSON object.

#### Validation
JSON input mostly comes via HTTP from untrusted sources.  Consequently, there is a need for validating the JSON
input.  Basic support for input validation is provided via a `validate` function.

A `validate( const char*, M& )` function is defined.  This is to for validating the JSON input being parsed. A
default specialisation is provided for `std::string` fields.  This rejects strings with more than `40%` (configurable)
special characters.  Users are advised to implement specific implementations specific to their domain requirements.
Users may also use environment variables to influence the default implementation.
* `JSON_PARSE_VALIDATION_IGNORE` - Environment variable that expects a comma or space separated list of
  field names that should be ignored by the validator.  Default values are `password, version`.  Example:
  `export SPT_JSON_PARSE_VALIDATION_IGNORE='password, file, version, firmware, identifier'`
* `SPT_JSON_PARSE_VALIDATION_RATIO` - Environment variable (`double`) that sets the maximum allowed ratio of
  special characters in the input string.  Default is `0.4`.  Example: `export SPT_JSON_PARSE_VALIDATION_RATIO='0.35'`

## Build
Check out the sources and use `cmake` to build and install the project locally.

### UNIX
<tabs id="mongo-service-unix">
  <tab title="Boost" id="build-boost-unix">
    <include from="boost.topic" element-id="boost-unix"/>
  </tab>
  <tab title="mongocxx" id="build-mongocxx-unix">
    Install <a href="https://www.mongodb.com/docs/languages/cpp/">mongocxx</a> driver.
    <code-block lang="SHELL" collapsible="false">
export PREFIX=/opt/local
    </code-block>
    <include from="build-mongocxx.topic" element-id="build-mongocxx-unix"/>
  </tab>
  <tab title="project" id="build-mongo-service-unix">
    Check out, build and install the project.
    <code-block lang="SHELL" collapsible="true">
<![CDATA[
cd /tmp
git clone https://github.com/sptrakesh/mongo-service.git
cd mongo-service
cmake -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/opt/local \
  -DCMAKE_INSTALL_PREFIX=/opt/spt \
  -DBUILD_TESTING=OFF -S . -B build
cmake --build build -j12
sudo cmake --install build
]]>
    </code-block>
  </tab>
</tabs>

### Mac OS X
<tabs id="mongo-service-macosx">
  <tab title="Boost" id="build-boost-macosx">
    <include from="boost.topic" element-id="boost-macosx"/>
  </tab>
  <tab title="mongocxx" id="build-mongocxx-macosx">
    Install <a href="https://www.mongodb.com/docs/languages/cpp/">mongocxx</a> driver.
    <code-block lang="SHELL" collapsible="false">
export PREFIX=/usr/local/mongo
    </code-block>
    <include from="build-mongocxx.topic" element-id="build-mongocxx-unix"/>
  </tab>
  <tab title="project" id="build-mongo-service-macosx">
    Check out, build and install the project.
    <code-block lang="SHELL" collapsible="true">
<![CDATA[
cd /tmp
git clone https://github.com/sptrakesh/mongo-service.git
cd mongo-service
cmake -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/usr/local/boost \
  -DCMAKE_PREFIX_PATH=/usr/local/mongo \
  -DCMAKE_INSTALL_PREFIX=/opt/spt \
  -DBUILD_TESTING=OFF -S . -B build
cmake --build build -j12
sudo cmake --install build
]]>
    </code-block>
  </tab>
</tabs>

### Windows

Install dependencies to build the project.  The following instructions at times reference `arm` or `arm64` architecture.  Modify
those values as appropriate for your hardware.  These instructions are based on steps I followed to set up the project on a
Windows 11 virtual machine running via Parallels Desktop on a M2 Mac.

<tabs id="mongo-service-windows">
  <tab title="Boost" id="build-boost-windows">
    <include from="boost.topic" element-id="boost-windows"/>
  </tab>
  <tab title="mongocxx" id="build-mongocxx-windows">
    Install <a href="https://www.mongodb.com/docs/languages/cpp/">mongocxx</a> driver.
    <include from="build-mongocxx.topic" element-id="build-mongocxx-windows"/>
  </tab>
  <tab title="fmt" id="build-fmt-windows">
    <include from="build-ranges.topic" element-id="build-fmt-windows"/>
  </tab>
  <tab title="ranges" id="build-ranges-windows">
    <include from="build-ranges.topic" element-id="build-ranges-windows"/>
  </tab>
  <tab title="vcpkg" id="install-vcpkg-windows">
    Install <a href="https://github.com/Microsoft/vcpkg">vcpkg</a> manager.  Launch the Visual Studio Command utility.
    <code-block lang="PowerShell" collapsible="true">
cd \opt\src
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat -disableMetrics
.\vcpkg integrate install --vcpkg-root \opt\src\vcpkg
.\vcpkg install curl:arm64-windows
.\vcpkg install cpr:arm64-windows
    </code-block>
  </tab>
  <tab title="project" id="build-mongo-service-windows">
    Check out, build and install the project.
    <code-block lang="PowerShell" collapsible="true">
<![CDATA[
cd %homepath%\source\repos
git clone https://github.com/sptrakesh/mongo-service.git
cd mongo-service
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=\opt\local -DCMAKE_INSTALL_PREFIX=\opt\spt -DBUILD_TESTING=ON -DCMAKE_TOOLCHAIN_FILE="C:/opt/src/vcpkg/scripts/buildsystems/vcpkg.cmake" -S . -B build
cmake --build build -j8
cmake --build build --target install
]]>
    </code-block>
  </tab>
</tabs>

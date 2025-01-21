# Serialisation
A simple serialisation framework is also provided.  Uses the
[visit_struct](https://github.com/cbeck88/visit_struct) library to automatically serialise and deserialise
*visitable* classes/structs.

## BSON
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

## JSON
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

### Validation
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

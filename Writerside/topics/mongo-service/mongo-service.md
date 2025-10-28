# Mongo Service

* [Models](mongo-service-models.md)
* [Serialisation Framework](mongo-service-serialisation.md)
* [Build](mongo-service-build.md)
* [Metrics](mongo-service-metrics.md)

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

## Clients
Client libraries are provided for the following languages:
* [C++](https://github.com/sptrakesh/mongo-service/tree/master/src/api)
* [Rust](https://github.com/sptrakesh/mongo-service/tree/master/client/rust/README.md)
* [Python](https://github.com/sptrakesh/mongo-service/tree/master/client/python)
* [Julia](https://github.com/sptrakesh/mongo-service/tree/master/client/julia/MongoService)

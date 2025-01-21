# Mongo Service Models
Model structures for making the various types of requests, and receiving the expected
responses are also provided.  Request and response structures accept user define data
models subject to some constraints.  See the model [unit test](https://github.com/sptrakesh/mongo-service/blob/master/test/unit/document.cpp)
suite for details on using the provided models.
* [api](https://github.com/sptrakesh/mongo-service/blob/master/src/api/api.hpp) lower level API for interacting with the service.
* [request](https://github.com/sptrakesh/mongo-service/tree/master/src/api/model/request) root folder with the
  structures that conform to the [protocol](mongo-service.md#protocol).  Models that accept references to user defined data are
  also available when creating/updating data.  See `CreateWithReference` structure in
  [create.hpp](https://github.com/sptrakesh/mongo-service/blob/master/src/api/model/request/create.hpp) for instance.
* [response](https://github.com/sptrakesh/mongo-service/tree/master/src/api/model/response) root folder with structures
  that conform to the service responses.
* [respository](https://github.com/sptrakesh/mongo-service/blob/master/src/api/repository/repository.hpp) repository
  interface for interacting with the service.  See [integration](https://github.com/sptrakesh/mongo-service/blob/master/test/integration/repository.cpp)
  test suite for examples of usage.

## Create
Protocol for creating documents in a collection.

<tabs id="mongo-service-protocol-create">
  <tab title="Request" id="mongo-service-protocol-create-request">
    Data model that represents the payload to be submitted to the service for creating documents.  Two structures are defined: `Create` that owns the document payload, and `CreateWithReference` that references instances that are owned by calling code.
    <code-block lang="C++" src="mongo/service/request/create.hpp" collapsible="true"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-create-response">
    Data model that represents the payload that the service responds with when creating a document.
    <code-block lang="C++" src="mongo/service/response/create.hpp" collapsible="true"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-create-example">
    Sample code illustrating the create action.
    <code-block lang="C++" src="mongo/service/example/create.cpp" collapsible="true"/>
  </tab>
</tabs>

## Create Timeseries
The `document` as specified will be inserted into the specified `database` and
timeseries `collection`.  The **BSON ObjectId** property/field (`_id`) may be omitted
in the document.  The response will include the server generated `_id` for the inserted document
if using *acknowledged* writes.  No version history is created for timeseries data.

<tabs id="mongo-service-protocol-create-timeseries">
  <tab title="Request" id="mongo-service-protocol-create-timeseries-request">
    Data model that represents the payload to be submitted to the service for creating documents.  Two structures are defined: `Create` that owns the document payload, and `CreateWithReference` that references instances that are owned by calling code.
    <code-block lang="C++" src="mongo/service/request/createtimeseries.hpp" collapsible="true"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-create-timeseries-response">
    Data model that represents the payload that the service responds with when creating a document.
    <code-block lang="C++" src="mongo/service/response/create.hpp" collapsible="true"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-create-timeseries-example">
    Sample code illustrating the create action.
    <code-block lang="C++" src="mongo/service/example/createtimeseries.cpp" collapsible="true"/>
  </tab>
</tabs>

## Retrieve
Protocol for retrieving document(s).

<tabs id="mongo-service-protocol-retrieve">
  <tab title="Request" id="mongo-service-protocol-retrieve-request">
    Data model that represents the payload to be submitted to the service for retrieving documents.  Model supports raw BSON document queries as well as structures that can be serialised to BSON.
    <code-block lang="C++" src="mongo/service/request/retrieve.hpp" collapsible="true"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-retrieve-response">
    Data model that represents the payload that the service responds with when retrieving documents.
    <code-block lang="C++" src="mongo/service/response/retrieve.hpp" collapsible="true"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-retrieve-example">
    Sample code illustrating the retrieve action.
    <code-block lang="C++" src="mongo/service/example/retrieve.cpp" collapsible="true"/>
  </tab>
</tabs>

## Count
Protocol for counting documents that match a filter.

<tabs id="mongo-service-protocol-count">
  <tab title="Request" id="mongo-service-protocol-count-request">
    Data model that represents the payload to be submitted to the service for retrieving documents.  Model supports raw BSON document queries as well as structures that can be serialised to BSON.
    <code-block lang="C++" src="mongo/service/request/count.hpp" collapsible="true"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-count-response">
    Data model that represents the payload that the service responds with when returning count of documents.
    <code-block lang="C++" src="mongo/service/response/count.hpp" collapsible="true"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-count-example">
    Sample code illustrating the count action.
    <code-block lang="C++" src="mongo/service/example/count.cpp" collapsible="true"/>
  </tab>
</tabs>

## Distinct
Protocol for retrieving distinct values of a specific field.  The payload document *must* contain
the `field` for which distinct values are to be retrieved.  An optional `filter` field can be used to specify the
filter query to use when retrieving distinct values.

<tabs id="mongo-service-protocol-distinct">
  <tab title="Request" id="mongo-service-protocol-distinct-request">
    Data model that represents the payload to be submitted to the service for retrieving distinct values.  Model supports raw BSON document queries as well as structures that can be serialised to BSON.
    <code-block lang="C++" src="mongo/service/request/distinct.hpp" collapsible="true"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-distinct-response">
    Data model that represents the payload that the service responds with when returning distinct values.
    <code-block lang="C++" src="mongo/service/response/distinct.hpp" collapsible="true"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-distinct-example">
    Sample code illustrating the distinct action.
    <code-block lang="C++" src="mongo/service/example/distinct.cpp" collapsible="true"/>
  </tab>
</tabs>

## Update
Update is the most complex scenario.  The service supports the two main update *modes* supported by Mongo:
* **update** - The data specified in the `document` sub-document is merged into the existing document(s).
* **replace** - The data specified in the `document` is used to replace an existing document.

Updates are possible either by an explicit `_id` field in the input `document`,
or via a `filter` sub-document that expresses the query used to identify the
candidate document(s) to update.

The returned BSON document depends on whether a single-document or multi-document update request was made:
* *Single-Document* - For single document updates the full updated stored document
  (`document`) and basic information about the associated version history document
  (`history`) are returned.
* *Multi-Document* - For multi-document updates, an array of BSON object ids for
  successful updates (`success`), failed updates (`failure`), and the basic
  information about the version history documents (`history`).

<tabs id="mongo-service-protocol-update">
  <tab title="Request" id="mongo-service-protocol-update-request">
    Data models that represents the payload to be submitted to the service for updating document(s).  Provides structures for updating a single document by merging, for replacing the stored document, as well as a structure for updating multiple documents.   Variants of the structures for owned vs referenced documents are also provided.
    <code-block lang="C++" src="mongo/service/request/update.hpp" collapsible="true"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-update-response">
    Data models that represents the payloads the service responds with when updating document(s).
    <code-block lang="C++" src="mongo/service/response/update.hpp" collapsible="true"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-update-example">
    Sample code illustrating the update actions.
    <code-block lang="C++" src="mongo/service/example/update.cpp" collapsible="true"/>
  </tab>
</tabs>

## Delete
The `document` represents the *query* to execute to find the candidate documents
to delete from the `database`:`collection`.  The query is executed to retrieve
the candidate documents, and the documents removed from the specified
`database:collection`.  The retrieved documents are then written to the version
history database.

<tabs id="mongo-service-protocol-delete">
  <tab title="Request" id="mongo-service-protocol-delete-request">
    Data models that represents the payload to be submitted to the service for deleting document(s).
    <code-block lang="C++" src="mongo/service/request/delete.hpp" collapsible="true"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-delete-response">
    Data models that represents the payloads the service responds with when deleting document(s).
    <code-block lang="C++" src="mongo/service/response/delete.hpp" collapsible="true"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-delete-example">
    Sample code illustrating the delete action.
    <code-block lang="C++" src="mongo/service/example/delete.cpp" collapsible="true"/>
  </tab>
</tabs>

## Bulk Write
Bulk insert/delete documents.  Corresponding version history documents for
inserted and/or deleted documents are created unless `skipVersion` is specified.

Note, the individual actions in the request are executed *serially* by the service.
Individual requests in the batch may fail.  This does not lead to service breaking out
of the processing loop.

<tabs id="mongo-service-protocol-bulk">
  <tab title="Request" id="mongo-service-protocol-bulk-request">
    Data models that represents the payload to be submitted to the service for creating/deleting documents.
    <code-block lang="C++" src="mongo/service/request/bulk.hpp" collapsible="true"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-bulk-response">
    Data models that represents the payloads the service responds with when creating/deleting document(s).
    <code-block lang="C++" src="mongo/service/response/bulk.hpp" collapsible="true"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-bulk-example">
    Sample code illustrating the bulk action.
    <code-block lang="C++" src="mongo/service/example/bulk.cpp" collapsible="true"/>
  </tab>
</tabs>

## Transaction
Execute a sequence of actions in a **transaction**.  Nest the individual actions
that are to be performed in the **transaction** within the `document` sub-document.

The `document` in the payload **must** include an `items` *array* of documents.
Each document in the array represents the full specification for the *action* in
the *transaction*.  The *document* specification is the same as the
*document* specification for using the service.

The specification for the *action* document in the `items` array is:
* `action (string)` - The type of action to perform.  Should be one of `create|update|delete`.
* `database (string)` - The database in which the *step* is to be performed.
* `collection (string)` - The collection in which the *step* is to be performed.
* `document (document)` - The BSON specification for executing the `action`.
* `skipVersion (bool)` - Do not create version history document for this action.

<tabs id="mongo-service-protocol-transaction">
  <tab title="Request" id="mongo-service-protocol-transaction-request">
    Data models that represents the payload to be submitted to the service for creating/deleting documents.
    <code-block lang="C++" src="mongo/service/request/transaction.hpp" collapsible="true"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-transaction-response">
    Data models that represents the payloads the service responds with when creating/deleting document(s).
    <code-block lang="C++" src="mongo/service/response/transaction.hpp" collapsible="true"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-transaction-example">
    Sample code illustrating the transaction action.
    <code-block lang="C++" src="mongo/service/example/transaction.cpp" collapsible="true"/>
  </tab>
</tabs>

## Aggregation Pipeline
Retrieve aggregated information from the database.  The `document` in the payload **must** include a `specification`
*array* of documents which correspond to the `match`, `lookup` ... specifications for the aggregation pipeline
operation (*stage*).  The matching documents will be returned in a `results` array in the response.

<tabs id="mongo-service-protocol-pipeline">
  <tab title="Request" id="mongo-service-protocol-pipeline-request">
    Data models that represents the payload to be submitted to the service for retrieving aggregated information.
    <code-block lang="C++" src="mongo/service/request/pipeline.hpp" collapsible="true"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-pipeline-response">
    Data models that represents the payloads the service responds with when returning matching aggregated information.
    <code-block lang="C++" src="mongo/service/response/retrieve.hpp" collapsible="true"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-pipeline-example">
    Sample code illustrating the pipeline action.
    <code-block lang="C++" src="mongo/service/example/pipeline.cpp" collapsible="true"/>
  </tab>
</tabs>

## Create Index
Ensure appropriate index exists in the specified `database:collection`. The `document` represents
the specification for the *index* to be created. Additional options for the index (such as *unique*)
can be specified via the optional `options` sub-document.

<tabs id="mongo-service-protocol-create-index">
  <tab title="Request" id="mongo-service-protocol-create-index-request">
    Data models that represents the payload to be submitted to the service for retrieving aggregated information.
    <code-block lang="C++" src="mongo/service/request/index.hpp" collapsible="true"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-create-index-response">
    Data models that represents the payloads the service responds with when returning matching aggregated information.
    <code-block lang="C++" src="mongo/service/response/retrieve.hpp" collapsible="true"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-create-index-example">
    Sample code illustrating the index action.
    <code-block lang="C++" src="mongo/service/example/index.cpp" collapsible="true"/>
  </tab>
</tabs>

## Drop Index
Drop the specified index from the `database:collection`.  The `document` represents the *index*
specification. Additional options for the index (such as *write concern*) can be specified
via the optional `options` sub-document.

One of the following properties **must** be specified in the `document`:
* `name` - The `name` of the *index* to drop.  Should be a `string` value.
* `specification` - The full document specification of the index that was created.

<tabs id="mongo-service-protocol-drop-index">
  <tab title="Request" id="mongo-service-protocol-drop-index-request">
    Data models that represents the payload to be submitted to the service for retrieving aggregated information.
    <code-block lang="C++" src="mongo/service/request/dropindex.hpp" collapsible="true"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-drop-index-response">
    Data models that represents the payloads the service responds with when returning matching aggregated information.
    <code-block lang="C++" src="mongo/service/response/dropindex.hpp" collapsible="true"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-drop-index-example">
    Sample code illustrating the index action.
    <code-block lang="C++" src="mongo/service/example/dropindex.cpp" collapsible="true"/>
  </tab>
</tabs>

## Create Collection
Create a `collection` in the specified `database`.  If a `collection` already exists in the
`database` with the same *name*, an error is returned.  This is primarily useful when clients
wish to specify additional options when creating a collection (eg. create a timeseries collection).

<tabs id="mongo-service-protocol-create-collection">
  <tab title="Request" id="mongo-service-protocol-create-collection-request">
    Data models that represents the payload to be submitted to the service for retrieving aggregated information.
    <code-block lang="C++" src="mongo/service/request/createcollection.hpp" collapsible="true"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-create-collection-response">
    Data models that represents the payloads the service responds with when returning matching aggregated information.
    <code-block lang="C++" src="mongo/service/response/createcollection.hpp" collapsible="true"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-create-collection-example">
    Sample code illustrating the index action.
    <code-block lang="C++" src="mongo/service/example/createcollection.cpp" collapsible="true"/>
  </tab>
</tabs>

## Rename Collection
Rename a `collection` in the specified `database`.  If a `collection` already exists with the
`document.target` *name*, an error is returned.  The option to automatically drop a pre-existing
collection as supported by **MongoDB** is not supported.  For such cases, use the [Drop Collection](#drop-collection)
action prior to invoking this action.  Specify the *write concern* settings in the
optional `options` sub-document.

This is a potentially heavy-weight operation.  All *version history* documents for the specified
`database::collection` combination are also updated.  Version history document update is performed
*asynchronously*.  The operation enqueues an update operation to the version history documents, and
returns.  This can lead to queries against version history returning stale information for a short
period of time.

**Note**: Renaming the collection in all associated *version history* documents may be the *wrong* choice.
In chronological terms, those documents were associated with the previous `collection`.  Only future revisions
are associated with the renamed `target`.  However, this can create issues in terms of retrieval, or if
iterating over records for some other purpose, or if a new collection with the *previous* name is created
in future.

<tabs id="mongo-service-protocol-rename-collection">
  <tab title="Request" id="mongo-service-protocol-rename-collection-request">
    Data models that represents the payload to be submitted to the service for retrieving aggregated information.
    <code-block lang="C++" src="mongo/service/request/renamecollection.hpp" collapsible="true"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-rename-collection-response">
    Data models that represents the payloads the service responds with when returning matching aggregated information.
    <code-block lang="C++" src="mongo/service/response/createcollection.hpp" collapsible="true"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-rename-collection-example">
    Sample code illustrating the index action.
    <code-block lang="C++" src="mongo/service/example/renamecollection.cpp" collapsible="true"/>
  </tab>
</tabs>

## Drop Collection
Drop the specified collection and all its containing documents.  Specify an
empty `document` in the payload to satisfy payload requirements.  If you wish
to also remove all version history documents for the dropped collection, specify
`clearVersionHistory` `true` in the `document` (revision history documents
will be removed *asynchronously*). Specify the *write concern* settings in the
optional `options` sub-document.

<tabs id="mongo-service-protocol-drop-collection">
  <tab title="Request" id="mongo-service-protocol-drop-collection-request">
    Data models that represents the payload to be submitted to the service for retrieving aggregated information.
    <code-block lang="C++" src="mongo/service/request/dropcollection.hpp" collapsible="true"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-drop-collection-response">
    Data models that represents the payloads the service responds with when returning matching aggregated information.
    <code-block lang="C++" src="mongo/service/response/dropcollection.hpp" collapsible="true"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-drop-collection-example">
    Sample code illustrating the index action.
    <code-block lang="C++" src="mongo/service/example/dropcollection.cpp" collapsible="true"/>
  </tab>
</tabs>

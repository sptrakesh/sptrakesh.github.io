# Rename Collection
Rename a `collection` in the specified `database`.  If a `collection` already exists with the
`document.target` *name*, an error is returned.  The option to automatically drop a pre-existing
collection as supported by **MongoDB** is not supported.  For such cases, use the [Drop Collection](mongo-service-models-drop-collection.md)
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
    <code-block lang="C++" src="mongo/service/request/renamecollection.hpp" collapsible="false"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-rename-collection-response">
    Data models that represents the payloads the service responds with when returning matching aggregated information.
    <code-block lang="C++" src="mongo/service/response/createcollection.hpp" collapsible="false"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-rename-collection-example">
    Sample code illustrating the index action.
    <code-block lang="C++" src="mongo/service/example/renamecollection.cpp" collapsible="false"/>
  </tab>
</tabs>

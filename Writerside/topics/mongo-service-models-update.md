# Update
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
    <code-block lang="C++" src="mongo/service/request/update.hpp" collapsible="false"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-update-response">
    Data models that represents the payloads the service responds with when updating document(s).
    <code-block lang="C++" src="mongo/service/response/update.hpp" collapsible="false"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-update-example">
    Sample code illustrating the update actions.
    <code-block lang="C++" src="mongo/service/example/update.cpp" collapsible="false"/>
  </tab>
</tabs>

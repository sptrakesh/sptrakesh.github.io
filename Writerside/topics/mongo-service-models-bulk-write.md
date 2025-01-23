# Bulk Write
Bulk insert/delete documents.  Corresponding version history documents for
inserted and/or deleted documents are created unless `skipVersion` is specified.

Note, the individual actions in the request are executed *sequentially* by the service.
Individual requests in the batch may fail.  This does not lead to service breaking out
of the processing loop.

<tabs id="mongo-service-protocol-bulk">
  <tab title="Request" id="mongo-service-protocol-bulk-request">
    Data models that represents the payload to be submitted to the service for creating/deleting documents.
    <code-block lang="C++" src="mongo/service/request/bulk.hpp" collapsible="false"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-bulk-response">
    Data models that represents the payloads the service responds with when creating/deleting document(s).
    <code-block lang="C++" src="mongo/service/response/bulk.hpp" collapsible="false"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-bulk-example">
    Sample code illustrating the bulk action.
    <code-block lang="C++" src="mongo/service/example/bulk.cpp" collapsible="false"/>
  </tab>
</tabs>

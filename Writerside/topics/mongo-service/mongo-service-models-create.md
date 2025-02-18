# Create
Protocol for creating documents in a collection.

<tabs id="mongo-service-protocol-create">
  <tab title="Request" id="mongo-service-protocol-create-request">
    Data model that represents the payload to be submitted to the service for creating documents.  Two structures are defined: `Create` that owns the document payload, and `CreateWithReference` that references instances that are owned by calling code.
    <code-block lang="C++" src="mongo/service/request/create.hpp"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-create-response">
    Data model that represents the payload that the service responds with when creating a document.
    <code-block lang="C++" src="mongo/service/response/create.hpp"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-create-example">
    Sample code illustrating the create action.
    <code-block lang="C++" src="mongo/service/example/create.cpp"/>
  </tab>
</tabs>
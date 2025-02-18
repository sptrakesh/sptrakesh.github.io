# Create Index
Ensure appropriate index exists in the specified `database:collection`. The `document` represents
the specification for the *index* to be created. Additional options for the index (such as *unique*)
can be specified via the optional `options` sub-document.

<tabs id="mongo-service-protocol-create-index">
  <tab title="Request" id="mongo-service-protocol-create-index-request">
    Data models that represents the payload to be submitted to the service for retrieving aggregated information.
    <code-block lang="C++" src="mongo/service/request/index.hpp" collapsible="false"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-create-index-response">
    Data models that represents the payloads the service responds with when returning matching aggregated information.
    <code-block lang="C++" src="mongo/service/response/retrieve.hpp" collapsible="false"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-create-index-example">
    Sample code illustrating the index action.
    <code-block lang="C++" src="mongo/service/example/index.cpp" collapsible="false"/>
  </tab>
</tabs>

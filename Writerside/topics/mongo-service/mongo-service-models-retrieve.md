# Retrieve
Protocol for retrieving document(s).

<tabs id="mongo-service-protocol-retrieve">
  <tab title="Request" id="mongo-service-protocol-retrieve-request">
    Data model that represents the payload to be submitted to the service for retrieving documents.  Model supports raw BSON document queries as well as structures that can be serialised to BSON.
    <code-block lang="C++" src="mongo/service/request/retrieve.hpp" collapsible="false"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-retrieve-response">
    Data model that represents the payload that the service responds with when retrieving documents.
    <code-block lang="C++" src="mongo/service/response/retrieve.hpp" collapsible="false"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-retrieve-example">
    Sample code illustrating the retrieve action.
    <code-block lang="C++" src="mongo/service/example/retrieve.cpp" collapsible="false"/>
  </tab>
</tabs>

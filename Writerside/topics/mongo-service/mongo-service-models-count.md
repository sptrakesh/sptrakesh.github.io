# Count
Protocol for counting documents that match a filter.

<tabs id="mongo-service-protocol-count">
  <tab title="Request" id="mongo-service-protocol-count-request">
    Data model that represents the payload to be submitted to the service for retrieving documents.  Model supports raw BSON document queries as well as structures that can be serialised to BSON.
    <code-block lang="C++" src="mongo/service/request/count.hpp" collapsible="false"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-count-response">
    Data model that represents the payload that the service responds with when returning count of documents.
    <code-block lang="C++" src="mongo/service/response/count.hpp" collapsible="false"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-count-example">
    Sample code illustrating the count action.
    <code-block lang="C++" src="mongo/service/example/count.cpp" collapsible="false"/>
  </tab>
</tabs>

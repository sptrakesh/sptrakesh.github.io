# Aggregation Pipeline
Retrieve aggregated information from the database.  The `document` in the payload **must** include a `specification`
*array* of documents which correspond to the `match`, `lookup` ... specifications for the aggregation pipeline
operation (*stage*).  The matching documents will be returned in a `results` array in the response.

<tabs id="mongo-service-protocol-pipeline">
  <tab title="Request" id="mongo-service-protocol-pipeline-request">
    Data models that represents the payload to be submitted to the service for retrieving aggregated information.
    <code-block lang="C++" src="mongo/service/request/pipeline.hpp" collapsible="false"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-pipeline-response">
    Data models that represents the payloads the service responds with when returning matching aggregated information.
    <code-block lang="C++" src="mongo/service/response/retrieve.hpp" collapsible="false"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-pipeline-example">
    Sample code illustrating the pipeline action.
    <code-block lang="C++" src="mongo/service/example/pipeline.cpp" collapsible="false"/>
  </tab>
</tabs>

# Distinct
Protocol for retrieving distinct values of a specific field.  The payload document *must* contain
the `field` for which distinct values are to be retrieved.  An optional `filter` field can be used to specify the
filter query to use when retrieving distinct values.

<tabs id="mongo-service-protocol-distinct">
  <tab title="Request" id="mongo-service-protocol-distinct-request">
    Data model that represents the payload to be submitted to the service for retrieving distinct values.  Model supports raw BSON document queries as well as structures that can be serialised to BSON.
    <code-block lang="C++" src="mongo/service/request/distinct.hpp" collapsible="false"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-distinct-response">
    Data model that represents the payload that the service responds with when returning distinct values.
    <code-block lang="C++" src="mongo/service/response/distinct.hpp" collapsible="false"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-distinct-example">
    Sample code illustrating the distinct action.
    <code-block lang="C++" src="mongo/service/example/distinct.cpp" collapsible="false"/>
  </tab>
</tabs>

# Create Collection
Create a `collection` in the specified `database`.  If a `collection` already exists in the
`database` with the same *name*, an error is returned.  This is primarily useful when clients
wish to specify additional options when creating a collection (eg. create a timeseries collection).

<tabs id="mongo-service-protocol-create-collection">
  <tab title="Request" id="mongo-service-protocol-create-collection-request">
    Data models that represents the payload to be submitted to the service for retrieving aggregated information.
    <code-block lang="C++" src="mongo/service/request/createcollection.hpp" collapsible="false"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-create-collection-response">
    Data models that represents the payloads the service responds with when returning matching aggregated information.
    <code-block lang="C++" src="mongo/service/response/createcollection.hpp" collapsible="false"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-create-collection-example">
    Sample code illustrating the index action.
    <code-block lang="C++" src="mongo/service/example/createcollection.cpp" collapsible="false"/>
  </tab>
</tabs>

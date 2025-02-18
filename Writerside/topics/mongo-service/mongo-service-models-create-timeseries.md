# Create Timeseries
The `document` as specified will be inserted into the specified `database` and
timeseries `collection`.  The **BSON ObjectId** property/field (`_id`) may be omitted
in the document.  The response will include the server generated `_id` for the inserted document
if using *acknowledged* writes.  No version history is created for timeseries data.

<tabs id="mongo-service-protocol-create-timeseries">
  <tab title="Request" id="mongo-service-protocol-create-timeseries-request">
    Data model that represents the payload to be submitted to the service for creating documents.  Two structures are defined: `Create` that owns the document payload, and `CreateWithReference` that references instances that are owned by calling code.
    <code-block lang="C++" src="mongo/service/request/createtimeseries.hpp" collapsible="false"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-create-timeseries-response">
    Data model that represents the payload that the service responds with when creating a document.
    <code-block lang="C++" src="mongo/service/response/create.hpp" collapsible="false"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-create-timeseries-example">
    Sample code illustrating the create action.
    <code-block lang="C++" src="mongo/service/example/createtimeseries.cpp" collapsible="false"/>
  </tab>
</tabs>

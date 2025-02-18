# Drop Index
Drop the specified index from the `database:collection`.  The `document` represents the *index*
specification. Additional options for the index (such as *write concern*) can be specified
via the optional `options` sub-document.

One of the following properties **must** be specified in the `document`:
* `name` - The `name` of the *index* to drop.  Should be a `string` value.
* `specification` - The full document specification of the index that was created.

<tabs id="mongo-service-protocol-drop-index">
  <tab title="Request" id="mongo-service-protocol-drop-index-request">
    Data models that represents the payload to be submitted to the service for retrieving aggregated information.
    <code-block lang="C++" src="mongo/service/request/dropindex.hpp" collapsible="false"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-drop-index-response">
    Data models that represents the payloads the service responds with when returning matching aggregated information.
    <code-block lang="C++" src="mongo/service/response/dropindex.hpp" collapsible="false"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-drop-index-example">
    Sample code illustrating the index action.
    <code-block lang="C++" src="mongo/service/example/dropindex.cpp" collapsible="false"/>
  </tab>
</tabs>

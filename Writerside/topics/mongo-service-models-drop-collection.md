# Drop Collection
Drop the specified collection and all its containing documents.  Specify an
empty `document` in the payload to satisfy payload requirements.  If you wish
to also remove all version history documents for the dropped collection, specify
`clearVersionHistory` `true` in the `document` (revision history documents
will be removed *asynchronously*). Specify the *write concern* settings in the
optional `options` sub-document.

<tabs id="mongo-service-protocol-drop-collection">
  <tab title="Request" id="mongo-service-protocol-drop-collection-request">
    Data models that represents the payload to be submitted to the service for retrieving aggregated information.
    <code-block lang="C++" src="mongo/service/request/dropcollection.hpp" collapsible="false"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-drop-collection-response">
    Data models that represents the payloads the service responds with when returning matching aggregated information.
    <code-block lang="C++" src="mongo/service/response/dropcollection.hpp" collapsible="false"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-drop-collection-example">
    Sample code illustrating the index action.
    <code-block lang="C++" src="mongo/service/example/dropcollection.cpp" collapsible="false"/>
  </tab>
</tabs>

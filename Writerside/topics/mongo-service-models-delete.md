# Delete
The `document` represents the *query* to execute to find the candidate documents
to delete from the `database`:`collection`.  The query is executed to retrieve
the candidate documents, and the documents removed from the specified
`database:collection`.  The retrieved documents are then written to the version
history database.

<tabs id="mongo-service-protocol-delete">
  <tab title="Request" id="mongo-service-protocol-delete-request">
    Data models that represents the payload to be submitted to the service for deleting document(s).
    <code-block lang="C++" src="mongo/service/request/delete.hpp" collapsible="false"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-delete-response">
    Data models that represents the payloads the service responds with when deleting document(s).
    <code-block lang="C++" src="mongo/service/response/delete.hpp" collapsible="false"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-delete-example">
    Sample code illustrating the delete action.
    <code-block lang="C++" src="mongo/service/example/delete.cpp" collapsible="false"/>
  </tab>
</tabs>

# Transaction
Execute a sequence of actions in a **transaction**.  Nest the individual actions
that are to be performed in the **transaction** within the `document` sub-document.

The `document` in the payload **must** include an `items` *array* of documents.
Each document in the array represents the full specification for the *action* in
the *transaction*.  The *document* specification is the same as the
*document* specification for using the service.

The specification for the *action* document in the `items` array is:
* `action (string)` - The type of action to perform.  Should be one of `create|update|delete`.
* `database (string)` - The database in which the *step* is to be performed.
* `collection (string)` - The collection in which the *step* is to be performed.
* `document (document)` - The BSON specification for executing the `action`.
* `skipVersion (bool)` - Do not create version history document for this action.

<tabs id="mongo-service-protocol-transaction">
  <tab title="Request" id="mongo-service-protocol-transaction-request">
    Data models that represents the payload to be submitted to the service for creating/deleting documents.
    <code-block lang="C++" src="mongo/service/request/transaction.hpp" collapsible="false"/>
  </tab>
  <tab title="Response" id="mongo-service-protocol-transaction-response">
    Data models that represents the payloads the service responds with when creating/deleting document(s).
    <code-block lang="C++" src="mongo/service/response/transaction.hpp" collapsible="false"/>
  </tab>
  <tab title="Example" id="mongo-service-protocol-transaction-example">
    Sample code illustrating the transaction action.
    <code-block lang="C++" src="mongo/service/example/transaction.cpp" collapsible="false"/>
  </tab>
</tabs>

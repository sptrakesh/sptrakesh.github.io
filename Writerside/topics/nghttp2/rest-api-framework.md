# REST API Framework

A framework I have used to build a few REST APIs.  API is implemented using a few template
functions which handle common CRUD operations I typically use when creating REST APIs. 
Persistence likewise is handled through a few template functions.

## Server
The server is implemented using [nghttp2-asio](nghttp2-asio.md) with the [nghttp2-framework](nghttp2-framework.md)
providing more advanced routing and asynchronous processing.  The template functions uses
the [Serialisation](mongo-service-serialisation.md) framework for JSON serialisation.

<tabs id="rest-api-framework-server">
  <tab title="Template" id="resp-api-framework-template">
    Template functions for handling common CRUD requests for a REST API service.
    <code-block lang="C++" src="nghttp2/rest/handlers/template.hpp" collapsible="true"/>
  </tab>
  <tab title="Common" id="resp-api-framework-common">
    Utility functions used to send error responses etc.
    <code-block lang="C++" src="nghttp2/rest/handlers/common.hpp" collapsible="true"/>
  </tab>
  <tab title="Endpoints" id="resp-api-framework-endpoints">
    Sample showing adding API endpoint routes to the router.  Delegates request processing to the appropriate template functions.
    <code-block lang="C++" src="nghttp2/rest/handlers/routes.hpp" collapsible="true"/>
  </tab>
</tabs>

## Models
Common structures used to implement the REST API.  User defined structures are developed
as appropriate, and used as template types.

<tabs id="rest-api-framework-model">
  <tab title="Entities" id="resp-api-framework-model-entities">
    A template structure used to returns multiple entities as a response.  The structure provides information to the caller related to the availability of further *pages* of data.
    <code-block lang="C++" src="nghttp2/rest/model/entities.hpp" collapsible="true"/>
  </tab>
  <tab title="EntitiesQuery" id="resp-api-framework-model-query">
    A simple structure used to represent common query string parameters supported by the API.
    <code-block lang="C++" src="nghttp2/rest/model/entitiesquery.hpp" collapsible="true"/>
  </tab>
</tabs>

## Persistence
Data persistence is based on MongoDB, and uses the [mongo-service](mongo-service.md) to
provide access and common API for database access.  The template functions use the
[Models](mongo-service-models.md) provided by the API to perform CRUD operations on
user defined data structures.

<tabs id="rest-api-framework-db">
  <tab title="Repository" id="resp-api-framework-db-repository">
    Template functions for performing CRUD operations against the database.
    <code-block lang="C++" src="nghttp2/rest/db/repository.hpp" collapsible="true"/>
  </tab>
  <tab title="Id" id="resp-api-framework-db-id">
    A simple structure representing a BSON ObjectId that is to be used as a query filter.  Also illustrates using customer serialisation to specify nested fields using dot notation.
    <code-block lang="C++" src="nghttp2/rest/db/filter/id.hpp" collapsible="true"/>
  </tab>
  <tab title="Property" id="resp-api-framework-db-property">
    A simple structure representing a field in a document and its value to use as a query filter.
    <code-block lang="C++" src="nghttp2/rest/db/filter/property.hpp" collapsible="true"/>
  </tab>
  <tab title="Between" id="resp-api-framework-db-between">
    A structure representing a range of dates along with some additional criterion to use as a query filter.
    <code-block lang="C++" src="nghttp2/rest/db/filter/between.hpp" collapsible="true"/>
  </tab>
</tabs>

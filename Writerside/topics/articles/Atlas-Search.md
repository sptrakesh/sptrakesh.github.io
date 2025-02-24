# Atlas Search

Most companies need some kind of full-text search engine as part of their infrastructure.
In the past, most companies I worked with used [Elastic](https://www.elastic.co/).  When
we needed to introduce a search engine at my current employer, I saw an announcement that
[MongoDB Atlas](https://www.mongodb.com/atlas) was introducing their cloud search service.
We already use Atlas as our database of record, and the new feature just reduced the
complexity of our infrastructure a lot.  We have been using Atlas Search for the past couple
of years, and have been very happy with the results.

## Global Search
We needed the ability to search across all our data, or specified subsets (specified entities)
of our data.  I chose to create a single collection in the database that stored all the
*search documents*.  This model allows us to search across all or a subset of entities with
absolutely no issues.

The strategy used is to extract all text values and store in a `values` array in the 
document stored in the collection configured with search index.  For our purpose we also
needed a couple of additional array fields for very specific use cases - `length`, and 
`date`.  Similarly, we also have an optional `location` field, that allows *proximity*
searches across relevant entities.

## Document Structure
I developed a very simple document structure which represents all our entities.  The document
has the following fields:

* `metadata` - A standard structure that captures the `created` and `modified` (last modified)
  dates for an entity, along with the user who last edited the entity.
* `usernames` - An array of platform `username` values that represents the list of users
  who have edited the source document at any point in its history.  This allows us to quickly
  find all entities that were edited at any time by a user.  We use this to show the most
  recently edited entities by the current logged-in user.
* `type` - An enumeration that identifies the entity that is being indexed.  This is the
  field specified by API clients when they wish to restrict search results to a specified
  type(s) of entities.  This also helps callers to identity the type of entity that is
  included in the search results.
* `values` - An array of string values.  All string values are extracted (recursively, and
  as appropriate from referenced entities) from the source entity and stored in the array.
  Search queries are matched against the values in this array.
* `location` - An object that stores the geo-coordinates and postal address for an entity.
  Not all entities have a `location` property, so location search will naturally be limited
  to only those entities have a *location*.
* `length` - An array of length values.  Similar to location, only a subset of entities have
  `length` values.  Multiple length values can be associated with a single entity, which
  is why this is an array.
* `date` - An array of date values.  Similar to `location` and `length`, only a subset of
  entities have additional `date` values to index.
* `entity` - Un-indexed stored field that stores the entire source document.  The entity
  `_id` is also used as the `_id` of the search document.  This also has the effect of
  making it extremely simple to determine which *database:collection* an `_id` in our
  system belongs to.

With this structure, we are able to index all the properties that are of interest to us
across all our entities.  The stored `entity` field provides access to the source document
direct from the search index, without needing to reach back into the BSON document stored
in the search collection, or needing to reach back into the original *database:collection*.

### Search by specified properties
The above structure with a simple `values` array of strings, supports mainly a traditional
search strategy. Users provide words/phrases of interest to search, and can further hone
the results by combining words/phrases to form **AND**/**OR** conditions.  For our
purposes, this was sufficient.  It also simplifies the requirements on the UI side, since
there is no need to build an extremely complicated advanced search interface.

I recently helped another customer implement a similar global search feature.  They had
requirements to search by specified fields, hence the `values` array was modified to
hold an array of *key-value* pairs, instead of simple strings.  This model supports both
simple searches by words/phrases, or specific search by constraining matches to only
those that have the specified *key*.  The search mapping had to be modified to use
the [embeddedDocuments](https://www.mongodb.com/docs/atlas/atlas-search/field-types/embedded-documents-type/#std-label-bson-data-types-embedded-documents)
type.

## Mapping
We use a `static` mapping scheme for the documents in the collection.  We need only a
subset of the fields in the documents to be indexed.  We also store the `type` and
`entity` fields in the search index, to enable direct use of the results from the search
engine, without needing to go back to the database for the original entities.

<code-block lang="JSON" collapsible="true" src="atlas.json"/>

**Note:** We use the `storedSource` option to specify the fields we wish to be stored
in the search index.  We make use of this by specifying `returnStoredSource: true` in
the `$search` aggregation query.  This avoids expensive fetches of the full document
from the indexed collection, especially if there are further stages in the aggregation
pipeline.

We specify a `$project` stage to retrieve the stored fields, as well as the search
relevance.

<code-block lang="JSON" collapsible="true">
[
  {
    "$search":
    {
      "index": "default",
      "count": {"type": "total"},
      "compound": {...},
      "returnStoredSource": true,
      "sort": {...}
    }
  },
  {
    "$project":
    {
      "type": 1,
      "entity": 1,
      "meta": "$$SEARCH_META",
      "score": {"$meta": "searchScore"}
    }
  },
  {"$skip": 200},
  {"$limit": 100},
]
</code-block>

## API Model
A simple JSON structure is used as the interface for executing searches against the
search index.

* `query` An array of string values.  Each entry in the array is used to compose the search 
  query.  If the entry consists of multiple words, the search results will include documents 
  that match *any* word (`OR` query).  If multiple words are to be treated as a *phrase*, 
  enclose them in double quotes (`"`).  Each item in the array is treated as a required match 
  for the search (the rule about separate words being treated as an `OR` still applies), 
  allowing caller to compose an `AND` query involving multiple words/phrases.
* `exclude` Word or words that should **not** match the search results.  As with `query`,
  individual words are treated as an `OR` clause, allowing caller to specify multiple 
  words/phrases that *must not* match the search results.
* `range` Allows caller to optionally limit search results to only documents that were 
  `created` or `modified` within a specified time interval.  Range also supports an additional 
  `date` type which matches against an array of dates extracted from a few entities.
* `length` Allows caller to optionally limit search results to only documents that have a
  `length` value that falls within the specified `lower` and `upper` bounds.
* `entities` Allows caller to optionally restrict matches to only the specified domain 
  object *types*.  If not specified or empty, matches all types of domain objects.
* `username` Allows caller to optionally limit search results to only documents that were 
edited by the specified *user*.  This is primarily useful for forensics purposes.

### Example
The following example shows a hypothetical search scenario, where we want to find matches 
for both `stickney` and `black`, but not associated with `cust22`.  In addition, we want 
to restrict results to only documents that were edited by `deleteduser` .  We also only 
wish to see documents that `deleteduser` modified between `Jan 1 2022` and `Dec 31 2022`.
We are also only interested in **Catalog** and **WorkOrder** type domain objects.

<code-block lang="JSON" collapsible="true">
{
  "query": ["stickney", "black"],
  "exclude": "cust22",
  "username": "deleteduser",
  "range": {
    "type": "modified",
    "start": "2022-01-01T00:00:00.000Z",
    "end": "2022-12-31T00:00:00.000Z"
  },
  "entities": ["Catalog", "WorkOrder"]
}
</code-block>
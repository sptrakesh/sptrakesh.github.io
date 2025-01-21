# Mongo Service Metrics
Metrics are collected for all requests to the service (unless client specifies `skipMetric`).  Metrics may be
stored in MongoDB itself, or a service that supports the [ILP](https://docs.influxdata.com/influxdb/v2.7/reference/syntax/line-protocol/).

### MongoDB
Metrics are collected in the specified `database` and `collection` combination
(or their defaults).  No TTL index is set on this (as it is left to the user's
requirements).  A `date` property is stored to create a TTL index as required.

The schema for a metric is as follows:
```json
{
  "_id": {"$oid": "5fd4b7e55f1ba96a695d1446"},
  "action": "retrieve",
  "database": "wpreading2",
  "collection": "databaseVersion",
  "size": 88,
  "time": 414306,
  "timestamp": 437909021088978,
  "date": {"$date": 437909021},
  "application": "bootstrap"
}
```

* **action** - The database action performed by the client.
* **database** - The database against which the action was performed.
* **collection** - The collection against which the action was performed.
* **size** - The total size of the response document.
* **time** - The time in `nanoseconds` for the action (includes any interaction with version history).
* **timestamp** - The time since UNIX epoch in `nanoseconds` for use when exporting to other timeseries databases.
* **date** - The BSON date at which the metric was created.  Use to define a TTL index as appropriate.
* **application** - The application that invoked the service if specified in the
  request payload.

### ILP
Metrics may be stored in a time series database of choice that supports the ILP.  We have only tested
storing metrics in [QuestDB](https://questdb.io/). The `id`, `duration`, and `size` values are stored as *field sets* and 
the other values stored as *tag sets*. The *name* for the series (*measurement*) can be specified via
the command line argument, or will default to the name of the `metrics` collection.
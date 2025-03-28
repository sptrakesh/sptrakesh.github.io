# Application Performance Management

A variety of products, both commercial and opensource are available for
**A**pplication **P**erformance **M**anagement (**APM**).  Most of the available
products involve users *instrumenting* source code of applications to be
managed.  The instrumentation results in data being published to services
provided by the service vendor.

A customer needed a solution which did not involve publishing management data to
an external service.  They also did not want an expensive commercial solution.
After researching available options, I decided to roll out a very simple framework
that used existing infrastructure (mainly in terms of database).  The basic task
of instrumenting source code remains unchanged in scope.

## Infrastructure
The solution I developed stored APM data into **QuestDB** which the customer already
used as their **T**ime **S**eries **D**ata**b**ase (**TSDB**).  The primary mode of
bulk data ingestion into **QuestDB** is via the **I**nflux **L**ine **P**rotocol (**ILP**).

## Implementation
A simple data structure was developed that matches the **ILP** format.  Applications can
be instrumented as desired using the simple data structure, and published to **QuestDB**
over **ILP**.

**Note:** Most existing APM solutions use or support instrumentation using **OpenTelemetry**.
However, I decided against using it and developed the simple data structure to make the
task of ingestion into **QuestDB** seamless.  This does limit options for future migration
into another database/storage engine.

## APM Record

The following structure is used to capture APM records.  A parent APM record is created
at the start of a *business process* (eg. HTTP request processing cycle, an operation that
is performed by a daemon process etc.).  Additional process records are added to the
parent as appropriate to capture the functions that are invoked as part of the process
implementation.  The full record can then be serialised over **ILP** to a supported database.

<tabs id="apm-model">
  <tab title="Model" id="apm-model-definition">
    <code-block lang="C++" src="ilp/apmrecord.hpp" collapsible="true"/>
  </tab>
  <tab title="Implementation" id="apm-model-implementation">
    <code-block lang="C++" src="ilp/apmrecord.cpp" collapsible="true"/>
  </tab>
  <tab title="BSON" id="apm-model-bson">
    Utility functions to (de)serialise the full APM record from/to BSON.
    <code-block lang="C++" src="ilp/bson.cpp" collapsible="true"/>
  </tab>
</tabs>

## Instrument Code
The toughest part of capturing **APM** data lies in instrumenting source code to
capture APM data at the desired level of verbosity.  There is obviously a trade-off
in terms of the effort required to instrument source code, as well as the performance
degradation involved in capturing APM data.  Another big side effect of instrumenting
source code is that the code base becomes littered with all the APM instrumentation
instructions.

<tabs id="apm-instrument">
  <tab title="HTTP" id="apm-instrument-template">
    Template functions for HTTP request handling introduced earlier in <a href="rest-api-framework.md">REST API Framework</a> updated with instrumentation.  The additional wrapper lines of code around key instruction and calls to other functions (internal and external) can be noticed.
    <code-block lang="C++" src="ilp/template.hpp" collapsible="true"/>
  </tab>
  <tab title="Database" id="apm-instrument-repository">
    Template functions for data access introduced earlier in <a href="rest-api-framework.md">REST API Framework</a> updated with instrumentation.
    <code-block lang="C++" src="ilp/repository.hpp" collapsible="true"/>
  </tab>
  <tab title="Handler" id="apm-instrument-handler">
    Sample REST API endpoint handlers that create the APM record which is then passed to the function call chain.
    <code-block lang="C++" src="ilp/catalog.cpp" collapsible="true"/>
  </tab>
  <tab title="Response" id="apm-instrument-response">
    A custom HTTP response structure that is used by the application.  The APM record is saved to MongoDB via the <a href="mongo-service.md">mongo-service</a> proxy.
    <code-block lang="C++" src="ilp/response.cpp" collapsible="true"/>
  </tab>
  <tab title="Sample" id="apm-instrument-sample">
    A sample APM record document generated during an integration test run from my local MongoDB instance.
    <code-block lang="JSON" src="ilp/apmrecord.json" collapsible="true"/>
  </tab>
</tabs>

### General Procedure
We follow the following general procedure for capturing **APM** data.
* Create an **APM Record** at the start of a business process cycle.  For our REST API handlers, we start by
  creating a record at the start of the handler function, and finish it (typically setting the duration) at
  the end of the function.  See lines `31`, `38` etc. in the *Handler* tab above.
* All our interfaces were modified to accept an additional `APMRecord` parameter.  We then pass the APM record
  all the way through the function call chain.
* We wrap each function invocation in a `WRAP_CODE_LINE` macro (see the *Handler* tab above).  This ensures 
  that each invocation results in the caller function being added to the `APMRecord::Process` record.
* At the start of each function, we add a `APMRecord::Process` to the apm record.
* We call the `setDuration` function at the end of each function (via a `DEFER` macro - see [defer.hpp](https://github.com/sptrakesh/mongo-service/blob/master/src/common/util/defer.hpp)).
* We add notes, errors etc. as appropriate to the process record to further enhance the data in the APM database.
* At the end of the business process cycle, we *publish* the APM data.
* See *Sample* tab above for a sample of the APM data collection in our application during a HTTP request process.

## Publish Data
The gathered APM data needs to be published to a supported database for requisite analysis.  There are a variety of options
available for publishing the data.  Some of the options I considered were the following:
* Publish the APM records directly over **ILP**.  This generally requires maintaining a connection pool of **ILP** clients.
* Write the individual APM records as simple text files to a directory, and have another process monitor the
  directory and publish each batch of records to QuestDB over **ILP**.
  * A small variation of the above.  Instead of writing to a local (generally a mounted) directory, to allow another
  process running on another node to access the files; we store the files into a cloud storage service such as AWS S3.
* Collect APM records for set periods of time in a single file (hourly for instance), and have the other process read 
  these files.  This is purely to improve write efficiency, as the services do not need to create individual files for
  each APM set.  This of course will need the use of mutexes or similar to ensure thread-safety.  This model would not
  work for storing in a cloud blob store.
* Write the APM records to an intermediary database or message queue, from which another process can republish
  the data over **ILP**.  This suffers from a potential issue in that storing to a local file is more fail-proof
  than writing to an external database.
  * For my current project, we decided to write to MongoDB.  We already had basic HTTP request metrics being collected
  and stored in MongoDB, before eventually being stored in QuestDB.  We decided to maintain the same strategy, while
  replacing the metric with the more comprehensive APM data.  There are a number of reasons for our choice:
    * QuestDB is not set up in a very robust manner.  Just a docker container running on a AWS EC2 host alongside
    other daemon processes.  There is no backup strategy.  All the data can be recreated from our Atlas database, which
    is our database of record.
    * All applications/services have a connection pool to [mongo-service](mongo-service.md).  There is no
    need to maintain yet another connection pool to QuestDB, which would also result in needing to increase the number
    of client connections it needs to support.
    * No need for other infrastructure such as AWS EFS to allow processes running on other nodes to access files written
    to by the services.
    * This strategy does suffer from the following downsides:
      * The risk of losing some APM records is higher, since the possibility of encountering
      database errors is slightly higher than the possibility of encountering filesystem errors.
      * Writing to an external database will be slower than writing to a local file.
      * We need another process than monitors the collection to which the APM records are being written to, and then publish those to
      QuestDB.  This is not a major downside for us, since we already have a suite of such monitors running for other
      business purposes.

## View Data
Data stored in QuestDB can be viewed by any tool that supports the PostgreSQL wire protocol.  We normally use
[Grafana](https://grafana.com/) for monitoring/analytics, and use the same to display the APM data.

<seealso>
  <category ref="source">
    <a href="https://github.com/sptrakesh/mongo-service/tree/master/src/ilp">ILP framework</a>
  </category>
  <category ref="external">
    <a href="https://questdb.com/">QuestDB</a>
    <a href="https://skywalking.apache.org/">Apache SkyWalking</a>
    <a href="https://newrelic.com/">new relic</a>
    <a href="https://www.datadoghq.com/">Datadog</a>
    <a href="https://opentelemetry.io/">OpenTelemetry</a>
  </category>
</seealso>
# Docker Container Statistics

We needed to get docker container statistics into a Grafana dashboard.  The
easiest option was to get the statistics published periodically to a QuestDB
instance, from which a Grafana dashboard was set up.

## Implementation
A simple Rust [project](https://gist.github.com/sptrakesh/1a8534d5aae3a526f5d38930c82d14e9)
was created to poll and publish the docker statistics.
The executable is linked against `musl libc` to get a fully *statically linked*
binary.  This allows us to copy the binary to the relevant AWS EC2 instances
without having to worry about installing any dependencies.

The information is gathered by executing `docker stats`, and the information converted to
ILP format and published to QuestDB.

<tabs id="container-stats">
  <tab title="Cargo" id="container-stats-cargo">
    The <code>cargo</code> configuration file for the project.
    <code-block lang="toml" src="container-stats/Cargo.toml" collapsible="false"/>
  </tab>
  <tab title="Main" id="container-stats-main">
    The process entry point implementation.  Sets up simple command line arguments, and retrieves the statistics for currently running containers.
    <code-block lang="Rust" src="container-stats/src/main.rs" collapsible="true"/>
  </tab>
  <tab title="Model" id="container-stats-model">
    Structures used to represent the docker statistics information.  A structure captures the raw information, which is then transformed into a processed structure, which is then converted to ILP.
    <code-block lang="Rust" src="container-stats/src/stats/mod.rs" collapsible="true"/>
  </tab>
  <tab title="ILP" id="container-stats-ilp">
    Module with a function that converts the parsed docker statistics information into ILP format and publishes to QuestDB.
    <code-block lang="Rust" src="container-stats/src/ilp/mod.rs" collapsible="true"/>
  </tab>
  <tab title="Build" id="container-stats-build">
    A simple script to build the executable that is deployed on our EC2 instances.  A commented line is shown which was used to install dependencies to build the application.
    <code-block lang="shell" src="container-stats/build.sh" collapsible="false"/>
  </tab>
</tabs>

## Deploy
The executable is copied to all the EC2 nodes that we run services/processes on, and
*systemd timer* created to run the process every 5 minutes.

```shell
[Unit]
Description=Gather docker container stats every 5 minutes
RefuseManualStart=no

[Timer]
Persistent=true
OnCalendar=*:5/5
Unit=container-stats.service

[Install]
WantedBy=timers.target
```

## QuestDB Table
The table schema as generated from the ILP data is shown below.
```
qdb=> show columns from containerStats;
      column       |   type    | indexed | indexBlockCapacity | symbolCached | symbolCapacity | designated | upsertKey 
-------------------+-----------+---------+--------------------+--------------+----------------+------------+-----------
 host              | SYMBOL    | f       |                  0 | t            |           1024 | f          | f
 container         | SYMBOL    | f       |                  0 | t            |           1024 | f          | f
 name              | SYMBOL    | f       |                  0 | t            |           1024 | f          | f
 id                | VARCHAR   | f       |                  0 | f            |              0 | f          | f
 cpu               | DOUBLE    | f       |                  0 | f            |              0 | f          | f
 memory_percentage | DOUBLE    | f       |                  0 | f            |              0 | f          | f
 pids              | LONG      | f       |                  0 | f            |              0 | f          | f
 memory_use        | DOUBLE    | f       |                  0 | f            |              0 | f          | f
 memory_use_unit   | VARCHAR   | f       |                  0 | f            |              0 | f          | f
 total_memory      | DOUBLE    | f       |                  0 | f            |              0 | f          | f
 total_memory_unit | VARCHAR   | f       |                  0 | f            |              0 | f          | f
 timestamp         | TIMESTAMP | f       |                  0 | f            |              0 | t          | f
 block_io_in       | DOUBLE    | f       |                256 | f            |              0 | f          | f
 block_io_in_unit  | VARCHAR   | f       |                256 | f            |              0 | f          | f
 block_io_out      | DOUBLE    | f       |                256 | f            |              0 | f          | f
 block_io_out_unit | VARCHAR   | f       |                256 | f            |              0 | f          | f
 net_io_in         | DOUBLE    | f       |                256 | f            |              0 | f          | f
 net_io_in_unit    | VARCHAR   | f       |                256 | f            |              0 | f          | f
 net_io_out        | DOUBLE    | f       |                256 | f            |              0 | f          | f
 net_io_out_unit   | VARCHAR   | f       |                256 | f            |              0 | f          | f
(20 rows)
```

## Grafana
A simple dashboard was created to view services and processes that we run.  Additional
dashboards and panels will be developed as needs arise.

<img src="container-stats.png" alt="Docker Container Statistics" thumbnail="true"/>
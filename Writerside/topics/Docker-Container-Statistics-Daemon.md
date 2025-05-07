# Docker Container Statistics Daemon

A *daemon* process that continually executes `docker stats`, aggregates the output and 
publishes to [QuestDB](https://questdb.com/) periodically. Statistics are continually polled
to ensure that we can capture temporary spikes, short scheduled job runs, etc.

This project builds on the work started in [Docker Container Statistics](Docker-Container-Statistics.md)
and enhances it to run as a persistent process, rather than a scheduled task that is
executed at the desired frequency (which only provides point-in-time information).

Optionally, the process also reports on the available free disk space for a configured
set of disks on the host machine.

## Command line arguments
The following arguments are supported for running the process.  Of these the
`node` argument is *required*, the others are optional as they have default values.

* `-n|--node` The host name to add to the published data.  Generally the name of the
  host docker daemon is running on.  This goes into the `host` column in QuestDB
  and can be used as variables in Grafana dashboards.
* `-m|--mode` The mode to use when publishing to QuestDB.  Defaults to `avg`.
  * `avg` Use average values for the aggregated data and publish the average periodically. 
  * `max` Pick the maximum value from the aggregated data and publish.  I primarily
    use this option, as I am more interested in seeing the spikes or worst cases scenarios
    being reported than in the averaged numbers.
* `-q|--questdb` The QuestDB host to publish to.  Defaults to `localhost`.
* `-s|--stats-table` The series name to publish to.  Defaults to `containerStats`.
* `-i|--interval` The interval in minutes for which statistics are aggregated.
  Defaults to `5` minutes.  Must be between `1` and `15` (I do not think larger
  values are useful, and there is not that much benefit in trying to reduce the
  amount of data stored in QuestDB).
* `-t|--transport-protocol` The QuestDB ILP transport protocol to use.  Default `tcp`.
  * `tcp` Publish to QuestDB over TCP/IP.
  * `http` Publish to QuestDB over HTTP.
  * `https` Publish to QuestDB over HTTPS.
* `-p|--port` The port on which the QuestDB ILP service is listening.  Default `9009`.
* `-w|--watchdog` *Linux only!*.  Enable or disable `systemd watchdog` notifications.
  If enabled, the systemd service unit **must** have `WatchdogSec` set.
  * `enabled` The default option.  Enable sending watchdog notifications at the
    recommended interval of roughly half the configured timeout value.
  * `disabled` Disable sending watchdog notifications.  This turns off all interactions
  with `libsystemd`.  Primarily meant to be used when the process is controlled via
  traditional [shell scripts](#alternate) or similar mechanisms, and not under `systemd`.
* `-b|--block-device` Optional list of disk/volume/block device names for which disk 
  usage statistics are to be captured.  Specify multiple times for multiple disks.
  `--block-device '/dev/nvme0n1p1' --block-device '/dev/nvme2n1'`
* `-d|--disk-table` The series name to publish disk information to.  Defaults to `diskStats`.

### Environment Variables
Process log level can be controlled using the standard `RUST_LOG` variable.  Process
logs at the `info` and `debug` levels.  For standard deployments, `info` level
logging should suffice. 

## Build
Standard `cargo` project, so build using `cargo build --release`.  For deployment,
I prepare a statically linked executable building for `aarch64-unknown-linux-musl`
target.  I usually build the deployment target on an Ubuntu VM running on Parallels.
There is also a [Dockerfile](https://github.com/sptrakesh/container-statsd/blob/main/docker/Dockerfile)
that can be used to build the executable.  Once the image has been built, run the 
image with a volume mount and copy the executable to the local directory.

**Note:** I do not use the `cargo` [package](https://pkgs.alpinelinux.org/packages)
as it is an older edition `2021` as opposed to the current `2024`.

```shell
docker run --rm -it -v "<full path to>:/tmp/mount" container-statsd
cp /opt/statsd/bin/container-statsd /tmp/mount/
```

## Run
The process will typically be run as a *service* through *systemd*.  The service
[unit](https://github.com/sptrakesh/container-statsd/blob/main/systemd/container-statsd.service)
sample file can be used as a template for setting up the service.

The latest executable for Linux on ARM64 can be downloaded from the project
[Releases](https://github.com/sptrakesh/container-statsd/releases) page.
The `container-statsd.bz2` file is the compressed executable.  This is the same file we
download on to our EC2 hosts and run via `systemd`.

**Note:** The user under whom the service is configured will need to be 
enabled as a *lingering user*.  If not, services started by the user will not
survive the user logging out.  This will also trigger an infinite stop/start
loop for the service (unless you configure the maximum number of restarts).

**Note:** The process publishes data using **ILP** over `TCP/IP`.  The QuestDB `TCP`
port (default `9009`) must be open and accessible from the hosts where the
process is installed.

```shell
sudo loginctl enable-linger <username>
mkdir -p ~/.config/systemd/user/
cp <path to checked out source>/systemd/container-statsd.service ~/.config/systemd/user/
systemctl --user daemon-reload
systemctl --user enable container-statsd.service
systemctl --user start container-statsd.service
systemctl status container-statsd.service
journalctl -f --user-unit container-statsd
```

### Alternate
A traditional shell-script approach may also be followed to run the process.
The following script can be used to control the daemon.

<code-block lang="SHELL" src="container-stats/run.sh" collapsible="true"/>

You can have a *systemd timer* check the process and restart if not running (we have
yet to run into an issue with the process aborting).

<include from="Docker-Container-Statistics.md" element-id="docker-stats-table"/>

### Disk Statistics Table
The optional disk statistics that are captured has the following schema:

```shell
qdb=> show columns from diskstats;
     column      |   type    | indexed | indexBlockCapacity | symbolCached | symbolCapacity | designated | upsertKey
-----------------+-----------+---------+--------------------+--------------+----------------+------------+-----------
 host            | SYMBOL    | f       |                  0 | t            |           1024 | f          | f
 name            | SYMBOL    | f       |                  0 | t            |           1024 | f          | f
 file_system     | SYMBOL    | f       |                  0 | t            |           1024 | f          | f
 mount_point     | SYMBOL    | f       |                  0 | t            |           1024 | f          | f
 type            | SYMBOL    | f       |                  0 | t            |           1024 | f          | f
 available_space | LONG      | f       |                  0 | f            |              0 | f          | f
 read_bytes      | LONG      | f       |                  0 | f            |              0 | f          | f
 write_bytes     | LONG      | f       |                  0 | f            |              0 | f          | f
 timestamp       | TIMESTAMP | f       |                  0 | f            |              0 | t          | f
 percentage_use  | DOUBLE    | f       |                256 | f            |              0 | f          | f
(10 rows)
```

**Note:** The `percentage_use` column is *mis-named*.  This column captures the
percentage of *available* space against the *total* space.  Hence, this column
captures the percentage available, and **not** the percentage *used*.  When
plotting the data, you will need to do a (100.0 - percentage_use) computation
in the query.

The image below shows IO (block device and network) statistics captured from
background processes (daemon and periodic) that run on our system.

<img src="container-stats-io.png" alt="IO Statistics" thumbnail="true"/>

The following image shows disk usage percentage from the optional feature.

<img src="container-stats-disk.png" alt="Disk Usage" thumbnail="true"/>
# Docker

Docker images are available on [Docker hub](https://hub.docker.com/repository/docker/sptrakesh/config-db).

The database files are stored under `/opt/spt/data`. If you wish to persist the database, 
volume mount this location.

It is possible to run the integration tests against the docker container instead of the service running locally.

```shell
docker run -d --rm -p 6026:6020 -p 2022:2020 --name config-db sptrakesh/config-db
# or with SSL turned on
docker run -d --rm -p 6026:6020 -p 2022:2020 -e "ENABLE_SSL=true" --name config-db sptrakesh/config-db
```

## Configuration
The following custom environment variables can be used to customise the container 
(the standard variables also work):

* `CONFIG_FILE` - The JSON file (volume mount) with full configuration. All other variables/options are ignored.
* `HTTP_PORT` - The port to run the HTTP/2 service on. Default `6020`.
* `TCP_PORT` - The port to run the TCP service on. Default `2020`.
* `NOTIFY_PORT` - The port to run the notification service on. Default `2120`
* `THREADS` - The number of threads to use for the services. Default `4`.
* `LOG_LEVEL` - The level to use for logging. One of `debug|info|warn|critical`. Default `info`.
* `ENABLE_CACHE` - Use to turn off temporary value caching. Default `true`.
* `ENABLE_SSL` - Use to run SSL services. Default `false`.
* `ENCRYPTION_SECRET` - Use to specify the secret used to *AES* encrypt values. Default is internal to the system.
* `PEERS` - Use to enable notifications on the `NOTIFY_PORT`. Listeners will be started to 
  listen for notifications from the `PEERS`. Will also enable publishing notifications
  from this instance.
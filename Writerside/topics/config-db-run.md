# Run

Run the service via the `/opt/spt/bin/configdb` executable. Command line options may be 
specified to override default options. When running as a *Docker* container, use environment 
variables to specify the command line options.

* `-f | --config-file` - The path to the JSON configuration file. All other options are 
  ignored. This option provides total control over all configurable options including 
  encryption. File must have same structure as the [struct](https://github.com/sptrakesh/config-db/blob/master/src/common/model/configuration.hpp).
  See [test](https://github.com/sptrakesh/config-db/blob/master/test/unit/configuration.cpp)
  for sample JSON configuration. For Docker specify the `CONFIG_FILE` environment variable.
* **Logging** - Options related to logging.
  * `-c | --console` - Flag that controls whether logs are echo'ed to stdout. Default is `off`. Always specified 
    in the Docker [entrypoint](https://github.com/sptrakesh/config-db/blob/master/docker/scripts/entrypoint.sh).
    No value (`true`, `false`) etc. must be specified.
  * `-l | --log-level` - The log level to set. Default `info`. Supported values `critical|warn|info|debug`.
    Specify via `LOG_LEVEL` environment variable to docker.
  * `-o | --log-dir` - The directory under which log files are written. Default `logs/`
    relative path. On docker this is set to `/opt/spt/logs`. Files are rotated daily. 
    External scripts (`cron` etc. are needed to remove old files).
* `-p | --http-port` - The port on which the HTTP/2 service listens. Default `6020` (`6026` on Apple).
  Specify via `HTTP_PORT` environment variable to docker.
* `-t | --tcp-port` - The port on which the TCP/IP service listens. Default `2020` (`2022`' on Apple).
  Specify via `TCP_PORT` environment variable to docker.
* `-b | --notify-port` - The port on which notifications are published. Default `2120` (`2122` on Apple).
  Specify via `NOTIFY_PORT` environment variable to docker.
* `-s | --with-ssl` - Flag to enable SSL on the HTTP/2 and TCP services.
  See [SSL](#ssl) for details. Specify via `ENABLE_SSL` environment variable to docker.
* `-n | --threads` - The number of threads for both TCP/IP and HTTP/2 services. Default to 
  number of hardware threads. Specify via `THREADS` environment variable to docker.
* `-e | --encryption-secret` - The secret to use to encrypt values. Default value is 
  internal to the system. Specify via `ENCRYPTION_SECRET` environment variable to docker.
* `-x | --enable-cache` - Flag to enable temporary cache for keys read from the database.
  Default `off`. Specify via `ENABLE_CACHE` environment variable to docker.
* `-z | --peers` - A comma separated list of peer instances to listen for notifications.
  Specify via the `PEERS` environment variable to docker. Eg. `localhost:2123,localhost:2124`

Sample command to run the service
```shell
# Locally built service
/opt/spt/bin/configdb --console --log-dir /tmp/ --threads 4 --log-level debug
# Docker container
docker run -d --rm -p 6020:6020 -p 2022:2020 \
  -e "ENCRYPTION_SECRET=svn91sc+rlZXlIXz1ZrGP4m3OgznyW5DrWONGjYw4bc=" -e "LOG_LEVEL=debug" \
  --name config-db config-db
```

## Environment Variables
Service [configuration](https://github.com/sptrakesh/config-db/blob/master/src/common/model/configuration.hpp)
can be customised using environment variables. All properties can be specified using a 
*snake case* convention with the common root `CONFIG_DB_` prefix.

* `CONFIG_DB_THREADS` - use to set the value of the `threads` field.
* `CONFIG_DB_ENABLE_CACHE` - use to set the value of the `enableCache` field. Set to 
  `false` to disable (default). Set to `true` to enable.
* `CONFIG_DB_ENCRYPTION_SALT` - use to set the value of the `encryption.salt` field.
* `CONFIG_DB_ENCRYPTION_KEY` - use to set the value of the `encryption.key` field.
* `CONFIG_DB_ENCRYPTION_IV` - use to set the value of the `encryption.iv` field.
* `CONFIG_DB_ENCRYPTION_SECRET` - use to set the value of the `encryption.secret` field.
* `CONFIG_DB_ENCRYPTION_ROUNDS` - use to set the value of the `encryption.rounds` field.
* `CONFIG_DB_LOGGING_LEVEL` - use to set the value of the `logging.level` field.
* `CONFIG_DB_LOGGING_DIR` - use to set the value of the `logging.dir` field.
* `CONFIG_DB_LOGGING_CONSOLE` - use to set the value of the `logging.console` field.
* `CONFIG_DB_SERVICES_HTTP` - use to set the value of the `services.http` field.
* `CONFIG_DB_SERVICES_TCP` - use to set the value of the `services.tcp` field.
* `CONFIG_DB_SERVICES_NOTIFY` - use to set the value of the `services.notify` field.
* `CONFIG_DB_PEERS[0..6]_HOST` - use to set the value of the `peers[n].host` field (e.g. `CONFIG_DB_PEERS0_HOST`).
* `CONFIG_DB_PEERS[0..6]_PORT` - use to set the value of the `peers[n].port` field.
* `CONFIG_DB_STORAGE_DBPATH` - use to set the value of the `storage.dbpath` field.
* `CONFIG_DB_STORAGE_BACKUP_PATH` - use to set the value of the `storage.backupPath` field.
* `CONFIG_DB_STORAGE_BLOCK_CACHE_SIZE_MB` - use to set the value of the `storage.blockCacheSizeMb` field.
* `CONFIG_DB_STORAGE_CLEAN_EXPIRED_KEYS_INTERVAL` - use to set the value of the `storage.cleanExpiredKeysInterval` field.
* `CONFIG_DB_STORAGE_ENCRYPTER_INITIAL_POOL_SIZE` - use to set the value of the `storage.encrypterInitialPoolSize` field.
* `CONFIG_DB_STORAGE_MAX_BACKUPS` - use to set the value of the `storage.maxBackups` field.
* `CONFIG_DB_STORAGE_USE_MUTEX` - use to set the value of the `storage.useMutex` field.

## Notifications
A notification system is available when services are run in a cluster. There is no complicated 
leadership election process (using Raft or similar) for cluster management/coordination. 
Each node is designed to run *stand-alone*. *Peer* instances are assumed to run independently 
(multi-master setup). A simple notification system has been implemented which will attempt 
to keep the independent nodes in *sync*. Notifications are primarily useful when using a 
cluster of instances that are also used as a *L1/L2* cache on top of application databases. 
Notifications are sent *asynchronously* and hence will only achieve eventual consistency 
in the best case.

When operating in *multi-master* mode, it is important to configure each instance with the 
appropriate list of peers. See [integration test](https://github.com/sptrakesh/config-db/blob/master/test/integration/multimaster.cpp)
for sample set up and test suite.

Each instance will publish updates (`Put`, `Delete`, and `Move`) via the notification service.
Corresponding listener instances will listen to notifications from the *peers*.

Notifications are **strictly** *one-way*. Notification service only *writes* messages to the 
socket, and the listener instances only *read* messages from the socket. Errors encountered 
while applying updates on another node do not have any effect on the publishing node.

Notification service sends periodic *ping* messages to keep the socket connections alive.

## SSL
SSL wrappers are enabled for both the TCP and HTTP/2 services. The package includes 
self-signed certificates. The [Makefile](https://github.com/sptrakesh/config-db/blob/master/certs/Makefile)
can be modified to generate self-signed certificates for your purposes. The easier way to
override the certificates is to volume mount the `/opt/spt/certs` directory when running 
the docker container.

At present, the file names are hard-coded:
* `ca.crt` - The root CA used to verify.
* `server.crt` - The server certificate file.
* `server.key` - The server key file.
* `client.crt` - The client certificate file.
* `client.key` - The client key file.

**Note:** There is definitely an overhead when using SSL. The integration test suite that 
ran in less than `100ms` now takes about `900ms`. With `4096` bit keys, it takes about `1.3s`.

**Note:** The TCP service only supports TLS `1.3`
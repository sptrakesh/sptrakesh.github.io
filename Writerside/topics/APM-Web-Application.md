# APM Web Application

A simple application built using [Wt](https://www.webtoolkit.eu/wt).  Similar to the 
[Desktop](APM-Desktop-Viewer.md) application, the primary purpose of this 
application is to display APM records as *forrests* that follow the function
call chain for instrumented services/applications.

This application is intended to deploy in the same network as [QuestDb](https://questdb.com/).
Unlike the desktop application, this application connects to QuestDB over the Postgres Wire Protocol,
to retrieve data.

## Features
The application provides two views:
* **List View** Display the parent APM records (generally those where the `type` column is `null`) in a table.
  The query used to retrieve the parent APM records is editable.  Change the query as appropriate to filter
  by additional conditions, time range, etc.
  Double-click a row to display a *tree view* with all the *child process* records for the parent APM record.
* ** Tree View** Display a tree with all the *child process* records for the latest parent APM records.  Select
  rows in the *tree* to view all available data for the record.

## Configuration
The application has very minimal configuration options.  The following environment variables can be used to
control the aspects that are configurable.
* `APM_USERNAME` The `username` to use to log in to the application.  Defaults to `apmuser`.
* `APM_PASSWORD` The `password` to use to log in to the application.
* `QUESTDB_URI` The PostgreSQL connection URI.  Default `postgresql://user:quest@localhost:8812/qdb`
* `APM_TABLE` The database table that holds the APM records.  Defaults to `webapm`.  Only one table is supported
  by the application at present.
* `SECURE_COOKIE` Specify `true` to set *secure* cookies (requires `https`).
  Cookies are used to track login sessions.

## Authentication
There is a rudimentary (and quite insecure) authentication layer for the application. This is just to ensure that
the application is not publicly accessible if deployed on the internet.  Ideally, the application is deployed in
a private network, and accessible only over VPN.  The `username` and `password` combination for logging in can
be controlled via environment variables.

## Docker
A docker [image](https://hub.docker.com/r/sptrakesh/apm-app) is available for the web application.

### Environment variables
The following variables can be used to customise the service.  The QuestDB URI
variable as specified earlier, except there is no default value.

* `QUESTDB_URI` The PostgreSQL connection URI.
* `PORT` Specify the port on which the server listens to.  Default `8000`.

It is recommended to volume mount a `/tmp/apm-sessions.json` file, which is used to track
sessions. This allows sessions to survive service restarts.

```shell
docker pull questdb/questdb
docker pull sptrakesh/apm-app
exists=`docker network ls | grep spt-net`
if [ -z "$exists" ]
then
  docker network create --driver bridge spt-net
fi
docker run -d --rm \
  -v "$HOME/tmp/spt/questdb:/var/lib/questdb" \
  -e QDB_PG_READONLY_USER_ENABLED=true \
  -p 9000:9000 -p 9009:9009 -p 8812:8812 -p 9003:9003 \
  --network spt-net \
  --name questdb questdb/questdb
touch $HOME/tmp/spt/apm-sessions.json
docker run --rm -d \
  -e "APM_USERNAME=apm" \
  -e "APM_PASSWORD=apm" \
  -e "QUESTDB_URI=postgresql://user:quest@questdb:8812/qdb" \
  -v "$HOME/tmp/spt/apm-sessions.json:/tmp/apm-sessions.json" \
  -p 8000:8000 \
  --network spt-net \
  --name apm-app \
  sptrakesh/apm-app
# ... after use
docker stop apm-app questdb
```

Once started, the application can be accessed at `http://localhost:8000/a`

## Screen captures
<img src="apm-web-login.png" alt="Login Screen" thumbnail="true"/>

<img src="apm-web-list.png" alt="List View" thumbnail="true"/>

<img src="apm-web-details.png" alt="APM Details" thumbnail="true"/>

<img src="apm-web-tree.png" alt="Tree View" thumbnail="true"/>

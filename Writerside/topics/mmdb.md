# MMDB

TCP and Websocket service for the [dbip](https://www.db-ip.com/db/download/ip-to-city-lite) MMDB Lite database.

## Protocol
Simplest use is to send the IP address for which you want details to the service,
and if it is a valid IP address, you will receive a JSON response with all the
details.

**Note:** For the **TCP** service, finish the `query` with a *single* UNIX newline
character (`\n`).  The response will be terminated by *two* UNIX newline characters
(`\n\n`).  Other than this, there is no difference between the **TCP** and
**Websocket** protocols.

## Example
The following example shows a request IP address and the response:

<code-block lang="json" src="mmdb.json" collapsible="true"/>

## Fields
If you prefer a simpler listing of *field : value* pairs, prefix the IP address
with `f:`.  The following example shows the same request as above:

```shell
f:184.105.163.155

city : Oklahoma City (Central Oklahoma City)
continent : North America
country : United States
country_iso_code : US
latitude : 35.524800
longitude : -97.597100
query_ip_address : 184.105.163.155
query_language : en
subdivision : Oklahoma
```

### Location
You can also retrieve the geo-coordinates (latitude and longitude) for an IP
address by prefixing the address with `l:`.  The following examples illustrates:

```shell
l:184.105.163.155

35.524800,-97.597100
```
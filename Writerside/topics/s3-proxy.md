# S3 Proxy
A simple proxy server for serving contents of an AWS S3 bucket as a static website.
Useful in cases where it is not feasible to use traditional CloudFront distribution
or similar CDN on top of a bucket. The server is implemented using 
[HTTP2 Framework](nghttp2-framework.md).

In particular, it is possible to serve static content from a private S3 bucket.

## Configuration
The server can be configured with the following options:
* `--auth-key` - A `bearer token` value to use to access internal management
  API endpoints.
  * At present the only management endpoint is `/_proxy/_private/_cache/clear`
    to clear the cache.
* `--key` - The *AWS Access key ID* to use to authorise the AWS SDK.
* `--secret` - The *AWS Secret access key* to use to authorise the AWS SDK.
* `--region` - The *AWS Region* in which the `bucket` exists.
* `--bucket` - The target *AWS S3 Bucket* from which objects are retrieved.
* `--port` - The *port* on which the server listens.  Default `8000`.
* `--threads` - Number of I/O threads for the server.  Defaults to `std::thread::hardware_concurrency`.
* `--ttl` - Cache expiry TTL in seconds.  Default `300`.  All S3 objects are
  cached for efficiency.
* `--cache-dir` - The directory under which cached S3 objects are stored.
  * Objects are stored under a filename that is the result of invoking
    `std::hash<std::string>` on the full path of the object.
  * Since hash values are used as filenames, the directory structure will be
    *flat*.  Makes it easy to have external jobs (eg. `cron` jobs) that delete
    the files on a regular basis.  No real need for it, unless a lot of files
    have been removed from the backing S3 bucket.
* `--reject-query-strings` - A flag to indicate that requests that contain query
  strings should be rejected as bad request.  Query strings in the context of serving
  out S3 resources may be viewed as a malicious request.
* `--console` - Set to `true` to echo log messages to `stdout` as well.  Default `false`.
* `--log-level` - Set the desired log level for the service.  Valid values are one of
  `critical`, `warn`, `info` or `debug`.
* `--dir` - Directory under which log files are to be stored.  Log files will be
  rotated daily.  Default `logs/` directory under the current working directory.
  * **Note:** if the directory does not exist, no logs will be output.
  * A trailing `/` is mandatory for the specified directory (eg. `/var/logs/proxy/`).
  * External scheduled job (`cron` or similar) that keeps a bound on the number of log
    files is required.
    * If run as a *Docker container* without the log directory being mounted
      as a volume, restarting the container will clear logs.  This will
      delete **all** historical logs.
    * Since we are dealing with a backing *S3 bucket*, the proxy will almost
      certainly be run on AWS infrastructure.  In this case, it is safe to
      restart the container, since the logs should end up in CloudWatch.
      * Ensure the *Docker* daemon has been configured to log to CloudWatch.
      * Ensure the server is started with `--console true`.
* `--mmdb-host` - If integration with [mmdb-ws](https://github.com/sptrakesh/mmdb-ws)
  is desired, specify the hostname for the service.
* `--mmdb-port` - Port on which the `mmdb-ws` service is listening.  Default is
  `2010`.  Only relevant if `--mmdb-host` is specified.
* `--ilp-host` - If metrics are to be published to an ILP (Influx Line Protocol) compatible database such as [QuestDB](https://questdb.io/).
  Works in combination with `--mmdb-host`.  Disabled if not specified.
* `--ilp-port` - Port on which *ILP* TCP service listens.  Default is `9009` (as used by QuestDB).
* `--ilp-series-name` - Name for the time series for metrics.  Default is `request`.
* `--mongo-uri` - If metrics are to be published to [MongoDB](https://mongodb.com/).
  Should follow [Connection String](https://docs.mongodb.com/manual/reference/connection-string/)
  URI format.  Works in combination with `--mmdb-host`.  Disabled if not specified.
* `--mongo-database` - Mongo *database* to write metrics to.  Default is `metrics`.
* `--mongo-collection` - Mongo *collection* to write metrics to.  Default is `request`.

Full list of options can always be consulted by running the following:

```shell
<path to>/s3proxy --help
# Install path specified for cmake is /opt/spt/bin
/opt/spt/bin/s3proxy --help
```

### Environment Variables
THe following environment variables may be used to specify additional configuration values to the
proxy server.
* `ALLOWED_ORIGINS` - JSON array of origins supported for sending CORS headers.  Also set this when
  running as a *docker container*.

```Shell
ALLOWED_ORIGINS='["http://127.0.0.1:8080", "http://localhost:8080", "http://local.sptci.com:8080"]' <path to>/s3proxy [options]
```

## Metric
Metric is captured in a simple [struct](https://github.com/sptrakesh/s3-proxy/blob/master/src/model/metric.h).  If **MMDB**
integration is enabled, additional information about the *visitors* geo-location
is retrieved and added to the metric.  The combined result is then saved to
**Time series database over ILP** and/or **MongoDB**.

### BSON Document
The following shows a sample `bson` document as generated by the integration
test suite.

```json
{
  "_id" : {"$oid": "5ee296de49d5eb77ef6580ef"},
  "method" : "GET",
  "resource" : "/index.html",
  "mimeType" : "text/html",
  "ipaddress" : "184.105.163.155",
  "size" : {"$numberLong": "2950"},
  "time" : {"$numberLong": "640224"},
  "status" : 200,
  "compressed" : false,
  "timestamp" : {"$numberLong" :"1591908061657005325"},
  "created" : {"$date": "2020-06-11T20:41:01.657Z"},
  "location" : {
    "query_ip_address" : "184.105.163.155",
    "continent" : "North America",
    "subdivision" : "Oklahoma",
    "query_language" : "en",
    "city" : "Oklahoma City (Central Oklahoma City)",
    "country" : "United States",
    "longitude" : "-97.597100",
    "country_iso_code" : "US",
    "latitude" : "35.524800"
  }
}
```

### Time Series
Metric as stored in the ILP compatible TSDB with multiple values.
in TSDB.  The following values are stored:
* `size` - A value used to track the response sizes sent.  **Note:**
  this value will differ based on whether `compressed` response was requested or not.
* `time` - A value used to track the time consumed to send the response in milliseconds.
* `status` - The HTTP response status.
* `latitude` - Latitude for the geo coordinates of the visitor.  This is only as accurate as the
  *IP Address* information inferred about the request.
* `longitude` - Longitude for the geo coordinates of the visitor.  This is only as accurate as the
  *IP Address* information inferred about the request.


#### Tags
Additional tags are stored using information in the metric.  These are highly variable,
hence choosing a TSDB that supports high-cardinality is required.  The following are some
of the tags that will be stored:
* `method` - The HTTP request method.
* `resource` - The path of the resource requested.
* `ipaddress` - The inferred IP address of the client making the request.
* `mimeType` - The MIME type of the resource requested.
* `city` - The city from which the requested originated.  Only works if MMDB was able to provide an address for IP address.
* `subdivision` - The subdivision from which the requested originated.  Only works if MMDB was able to provide an address for IP address.
* `country` - The country from which the requested originated.  Only works if MMDB was able to provide an address for IP address.
* `country_iso_code` - The ISO code for the country from which the requested originated.  Only works if MMDB was able to provide an address for IP address.
* `continent` - The continent from which the requested originated.  Only works if MMDB was able to provide an address for IP address.
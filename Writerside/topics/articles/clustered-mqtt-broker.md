# Clustered MQTT Broker
A custom MQTT broker developed to handle communications between IoT devices and cloud services.

Built using [mqtt_cpp](https://github.com/redboltz/mqtt_cpp) open-source project.

## Rationale
Initially a *websocket* based high performance and highly scalable services were developed 
to handle the communications. Unfortunately *websocket* libraries are not readily available 
in the *embedded C* space, and hence the sensor firmware development company recommended use 
of the popular [MQTT](http://mqtt.org/) protocol.

I set up an *AWS IoT* core service and also developed a very simple listener daemon on the 
cloud. This scheme works very well, but it opened up some questions about potential data 
loss, since **MQTT** is a very rudimentary *pub/sub* based protocol, and does not have strong 
loss-less message delivery guarantees that streaming protocols like **kafka** introduced.

To get around these limitations, I decided to implement our own **MQTT broker**, which 
would take care of message persistence and provide us the strong loss-less guarantee
that we were looking for.

## Implementation
I followed the same overall design that the *mqtt_cpp* project code samples and test 
suite used. A common **broker** implementation that implemented the various **MQTT**
protocol handlers, and two wrapping services - **TLS** and **non-TLS**.

An advantage of the custom broker implementation is that there is no need for each message 
from the **IoT device** to carry along the device identifier (a GUID assigned to the device 
by the device manufacturer or firmware). The *device identifier* is specified as the 
`clientId` when connecting to the **MQTT broker**. This information is normally not 
available to a *subscriber*, but since we are handling the messages on the *broker*, we 
have access to the `clientId`.

## Persistence
All messages received by the **broker** are persisted to the same backend **MongoDB** 
database that the rest of the cloud services use.

### Subscriptions
All *subscriptions* by clients are saved to the `mqtt.subscription` *collection*. This 
enables support for `cleanSession: false` when a **client** establishes a connection to the 
**broker**. 

The structure of a persisted subscription is as shown:
<code-block lang="JSON" collapsible="true">
{
  "_id" : {"$oid": "5d958499c88c526e030306c1"},
  "topic" : "/local/+/message",
  "clientId" : "listener:db:local",
  "wildcard": "+",
  "qos" : 1,
  "created" : {"$date": "2019-10-03T05:18:17.408Z"}
}
</code-block>

#### Wildcard Topics
Wildcard topics (`+` and `#`) are supported by pre-parsing the topic name and storing a 
`wildcard` field in the document. When retrieving the *subscriber* list to disseminate 
incoming *messages* to, *subscribers* that specified *topic* names with *wildcard patterns*
are retrieved and then compared for match with the message *topic*.

It would be possible to build a more optimal query by translating the wildcard topic 
pattern into a appropriate Mongo *regex* query, but for our usecase it is an overkill 
trying to implement a topic pattern as a Mongo *regex DSL*.

### Messages
Messages received by the **broker** are persisted to the `mqtt.message` collection. These 
documents have a *TTL index* set to purge automatically after `365` days.

The structure of a persisted message is as shown:

<code-block lang="JSON" collapsible="true">
{
  "_id" : {"$oid": "5d99d33f2a0886257e0014c8"},
  "topic" : "/coordinates",
  "data" : "66.6666,-88.8888",
  "clientId" : "MQTT_FX_Client",
  "qos" : 0,
  "retain" : true,
  "created" : {"$date": "2019-10-06T11:42:55.030Z"},
  "host" : "rakesh-mbp.local"
}
</code-block>

where `data` is the message payload. In our case, we only receive **CSV** messages, 
hence the payload is stored as a `string`. For more general purpose use, they would be 
saved as `binary` type.

### Published Messages
All published messages are persisted to the `mqtt.publishedMessage` collection. This 
allows tracking of messages that have been delivered to a *client*, and also ensures 
that every subscriber (even if off-line for a while) receives every message that was 
*published* to the *topics* of interest. These documents have a **TTL index** set to 
purge after a period of `30` days.

The structure of a published message looks like the following:
<code-block lang="JSON" collapsible="true">
{
  "_id" : {"$oid": "5d9b88935796f916e56c0cb4"},
  "clientId" : "MQTT_FX_Client",
  "topic" : "test",
  "messageId" : {"$oid": "5d9b885dffe3e3b8b9c2b1f9"},
  "host" : "rakesh-mbp.local",
  "created" : {"$date": "2019-10-07T18:48:51.142Z"}
}
</code-block>

## Clustering
All state - client subscriptions, messages, published messages etc. - are captured in the 
backend Mongo database. This allows **clustering** of the **brokers**, since messages received
on one **broker** can also be delivered to **clients** connected to other **brokers** in the 
**cluster** (assuming they are subscribed to the same topic).

When the **broker** receives either a `ping` or a `publish` request, the **broker** queries 
the database for *messages* published to *topics* the **client** has *subscribed* to from 
other hosts in the **cluster**. Any *messages* that were previously not published to the 
*client* are published at this stage.

> **Note**: When subscribing to a *topic* for the first time, only a *message* with `retain`
> flag set is *published*. Other messages published to the topic are not published.

## Security
As noted previously, the implementation supports operating with or without **TLS** enabled. 
If **TLS** support is required, the following types of certificates are needed:

* Server
  * Root CA. Certificate for the root certificate authority.
  * Server certificate. PEM format certificate identifying the server and the DNS name 
    for the server.
  * Server key - Certificate key file for the server.
* Client
  * Root CA. Same as the one used with server.
  * Client certificate. PEM format certificate for the client. Based on OpenSSL documentation and local testing, this seems optional.
  * Client key - Certificate key file for the client. As with client certificate this seems optional.

## Configuration
Configuration such as database connection information, certificate files, TLS etc. are 
configured via [config-db](https://github.com/sptrakesh/config-db) as the rest of the 
services in the cloud stack.

* `/iot/mqtt/tls` - Specify `true|false` to enable or disable **TLS**. Default false.
* `/iot/mqtt/cacert` - Path to Root CA certificate file.
* `/iot/mqtt/serverCert` - Path to server certificate file.
* `/iot/mqtt/serverKey` - Path to server certificate key file.

<img src="mqtt-broker.png" alt="MQTT Broker" border-effect="line" thumbnail="true"/>

<seealso>
  <category ref="mqtt-broker">
    <a href="https://www.bevywise.com/download_iot/makefile">Create Certificates</a>
    <a href="https://github.com/lelylan/haproxy-mqtt">Reverse Proxy</a>
    <a href="https://medium.com/@emqtt/deploying-emq-with-elastic-load-balancer-on-aws-21402322f711">AWS Load Balancer</a>
  </category>
</seealso>

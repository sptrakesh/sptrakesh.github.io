# Config Db

A simple [configuration database](https://github.com/sptrakesh/config-db) similar to Apache Zookeeper,
Etcd, etc., built using [RocksDB](https://rocksdb.org/).

All values are stored encrypted using `aes-256-cbc` on disk.

## Keys

All *keys* are assumed to represent UNIX *path like* structures. Keys that do not at least start with 
a leading `/` character are internally replaced with a leading `/` character. This means that keys 
`key` and `/key` are treated as identical.

The database maintains a hierarchical model of the *keys*. It is possible to list the child node names 
under each level in the hierarchy.

**Note:** When listing child node names, the full path of the parent *has* to be specified. Relative 
path names are not supported.

## Implementation

*Key-value* pairs are stored in a RocksDB *column family*. A few addition *column families* are used to
provide support for additional features provided.

* `data` - The main *column family* used to store the *key->value* mapping. The *value* is stored *encrypted*.
* `forrest` - The *column family* used to store the tree hierarchy.
* `expiration` - The *column family* used to store *TTL* values for *expiring* keys.
* `expiring` - The *column family* used to efficiently determine expired keys. The TTL value (along 
  with a value for disambiguation) is stored as the *key* and the data key is stored as the *value*
  in this column. RocksDB keys are stored sorted, hence using the *expiration time* as the *key*
  allows for efficiently breaking from the loop when the first non-expired key is encountered. A 
  custom *iterator* is used to retrieve only the *expired keys* from this column.
* `cache` - The column family used to indicate that the specified key is stored as a cache entry.

### Backup

Daily backups are performed and saved under the `storage.backupPath` configured. The maximum number 
of previous backups to maintain can be configured using the `storage.maxBackups` property. RocksDB 
makes incremental backups, and the cost of initialising the BackupEngine is proportional to the 
number of old backups.

### Peering

Each instance is designed to run independently. It is possible to configure instances to run as a 
*co-ordinated* cluster by configuring *peers* for each instance. There is no complicated leadership
election process/protocol using *Raft* or similar consensus algorithms. Commands received by each 
node is relayed to *peers*. This helps all *nodes* in the cluster remain *in sync* with each other 
eventually.
# Utilities

Utility applications to interact with the database are provided. These are deployed to the 
`/opt/spt/bin` directory.

## Shell

The `configsh` application provides a shell to interact with the database. The server should 
be running and the TCP port open for the application to connect.

The following command line options are supported by the shell:
* `-s | --server` The TCP server hostname to connect to. Default `localhost`.
* `-p | --port` The TCP port to connect to. Default `2020` (`2022` on Mac OS X).
* `-t | --with-ssl` Flag to connect over SSL to the server.
* `-l | --log-level` The level for the logger. Accepted values `debug|info|warn|critical`. Default `info`.
* `-o | --log-dir` The directory to write log files to. The path **must** end with a trailing `/`. Default `/tmp/`.

The following shows a simple CRUD type interaction via the shell.

<code-block lang="Shell" collapsible="true">
<![CDATA[
/opt/spt/bin/configsh --server localhost --port 2022 --log-level debug --log-dir /tmp/
Enter commands followed by <ENTER>
Enter help for help about commands
Enter exit or quit to exit shell
configdb> help
Available commands
  ls <path> - To list child node names.  Eg. [ls /]
  get <key> - To get configured value for key.  Eg. [get /key1/key2/key3]
  set <key> <value> - To set value for key.  Eg. [set /key1/key2/key3 Some long value. Note no surrounding quotes]
  mv <key> <destination> - To move value for key to destination.  Eg. [mv /key1/key2/key3 /key/key2/key3]
  rm <key> - To remove configured key.  Eg. [rm /key1/key2/key3]
  import <path to file> - To bulk import key-values from file.  Eg. [import /tmp/kvps.txt]
configdb> set /key1/key2/key3 {"glossary":{"title":"example glossary","GlossDiv":{"title":"S","GlossList":{"GlossEntry":{"ID":"SGML","SortAs":"SGML","GlossTerm":"Standard Generalized Markup Language","Acronym":"SGML","Abbrev":"ISO 8879:1986","GlossDef":{"para":"A meta-markup language, used to create markup languages such as DocBook.","GlossSeeAlso":["GML","XML"]},"GlossSee":"markup"}}}}}
Set key /key1/key2/key3
configdb> ls /
key1
configdb> ls /key1
key2
configdb> ls /key1/key2
key3
configdb> get /key1/key2/key3
{"glossary":{"title":"example glossary","GlossDiv":{"title":"S","GlossList":{"GlossEntry":{"ID":"SGML","SortAs":"SGML","GlossTerm":"Standard Generalized Markup Language","Acronym":"SGML","Abbrev":"ISO 8879:1986","GlossDef":{"para":"A meta-markup language, used to create markup languages such as DocBook.","GlossSeeAlso":["GML","XML"]},"GlossSee":"markup"}}}}}
configdb> set /key1/key2/key3 some long value with spaces and "quoted" text.
Set key /key1/key2/key3
configdb> get /key1/key2/key3
some long value with spaces and "quoted" text.
configdb> rm /key1/key2/key3
Removed key /key1/key2/key3
configdb> ls /
Error retrieving path /
configdb> import /tmp/import.txt
Imported (5/5) keys from file /tmp/import.txt
configdb> exit
Bye
]]>
</code-block>

## CLI

The `configctl` application provides a simple means for interacting with the database server.
Use it to execute single actions against the service when required.

**Note:** Unlike the **shell** application, `value`s specified as command line argument **must**
be *quoted* if they contain spaces or other special characters.

The server should be running and the TCP port open for the application to connect.

The following command line options are supported by the CLI application:
* `-s | --server` The TCP server hostname to connect to. Default `localhost`.
* `-p | --port` The TCP port to connect to. Default `2020` (`2022` on Mac OS X).
* `-t | --with-ssl` Flag to connect over SSL to the server.
* `-l | --log-level` The level for the logger. Accepted values `debug|info|warn|critical`. Default `info`.
* `-o | --log-dir` The directory to write log files to. The path **must** end with a trailing `/`. Default `/tmp/`.
* `-f | --file` The file to bulk import into the database. If specified, other commands are ignored.
* `-a | --action` The *action* to perform. One of `get|set|move|delete|list`.
* `-k | --key` The *key* to act upon.
* `-v | --value` The *value* to set. For `move` this is the destination path.

See [sample](https://github.com/sptrakesh/config-db/blob/master/test/integration/configctl.sh) integration test suite.

The following shows a simple CRUD type interaction via the cli. These were using the default values for
`server [-s|--server]` and `port [-p|--port]` options.

<code-block lang="Shell" collapsible="true">
spt:/home/spt $ /opt/spt/bin/configctl -a list -k /
Error retrieving path /
spt:/home/spt $ /opt/spt/bin/configctl -a set -k /test -v value
Set value for key /test
spt:/home/spt $ /opt/spt/bin/configctl -a list -k /            
test
spt:/home/spt $ /opt/spt/bin/configctl -a set -k /test -v value
Set value for key /test
spt:/home/spt $ /opt/spt/bin/configctl -a set -k /test -v "value modified"                      
Set value for key /test
spt:/home/spt $ /opt/spt/bin/configctl -a get -k /test 
value modified         
spt:/home/spt $ /opt/spt/bin/configctl -a move -k /test -v /test1
Moved key /test to /test1
spt:/home/spt $ /opt/spt/bin/configctl -a delete -k /test1
Removed key /test1
spt:/home/spt $ /opt/spt/bin/configctl -a list -k /
Error listing path /
</code-block>

### Bulk Import

A special `-f | --file` option is supported for bulk importing *key-value* pairs from a 
file. The input file **must** contain lines where each line represents a *key-value* pair.
The text before the first space character is interpreted as the *key*, and the rest of the
line as the *value*.

```shell
/a/b/c string value
/a/b/d 13
```

**Note:** The entire file is imported as a single *transaction*. The input file must be 
small enough to buffer the requests and transaction in memory.

<code-block lang="Shell" collapsible="true">
spt:/home/spt $ cat /tmp/import.txt
/a/b/c  a long string
/a/b/d another long value
/a/c/e  1234
/a/c/f 45765789
/a/c/g 123.347
spt:/home/spt $ /opt/spt/bin/configctl -f /tmp/import.txt
Set 5 keys
</code-block>

### Seed

A `seed-configdb` utility is provided to seed the database. This can be used to 
pre-populate the database before the service is started.

**Note:** This will fail if the service is already running (database locked by running process).

The following command line options are supported by the seed application:

* `-c | --conf` Optional JSON configuration file that controls the database storage location.
* `-f | --file` The data file to import. The input file **must** contain lines where each 
  line represents a *key-value* pair. The text before the *first space* character is 
  interpreted as the *key*, and the rest of the line as the *value*.
* `-l | --log-level` The level for the logger. Accepted values `debug|info|warn|critical`. Default `info`.
* `-o | --log-dir` The directory to write log files to. The path **must** end with a trailing `/`. Default `/tmp/`.
# Daily Rolling Logger

A logger that rolls over the output file on a daily basis. Modified the 
[lumberjack](https://github.com/natefinch/lumberjack) implementation to roll 
over based on date rather than file size. Supported options are mostly the same
as *lumberjack*:

* `file` - The fully qualified name for the output log file.
* `max` - The maximum number of old log files to preserve.
* `localTime` - Flag indicating whether to roll over at midnight in local time zone or **UTC**.
* `compress` - Flag indicating whether rolled over log files should be compressed (*gzip*) or not.
* `level` - Default log level to use when logging.

## Log Levels

Log levels follow the standard java util logging (*JULI*) levels (without going as deep as *JULI* does).

* `Severe` - Highest level for the logger.
* `Warning` - Next level
* `Info` - Normal and in general the default level.
* `Fine` - Generally used for debug level messages.
* `Finer` - Lowest level for the logger. May eventually remove and stop at Fine.

## Sample Use
<code-block lang="go" collapsible="true">
func main() {
  port := flag.Int("port", 8080, "Port for the service")
  etcd := flag.String("etcd", "localhost:2379", "etcd ensemble to use")
  logFile := flag.String("log", "", "Path to log file.  Default /tmp/server.log")
  console := flag.Bool("console", false, "Echo log output to console.  Default false")
  level := flag.String("logLevel", "Info", "Log level to use.  Default Info")
  localTime := flag.Bool("localTime", true, "Use local time or UTC for log rotation.  Default true")
  compress := flag.Bool("compress", true, "Compress rotated log files.  Default true")
  maxFiles := flag.Int("maxFiles", 15, "Maximum number of old log files to keep.  Default 15")
  flag.Parse()

  l := logger.NewLogger(*logFile, *maxFiles, *localTime, *compress, logger.LevelFromString(*level))

  if *console {
    mw := io.MultiWriter(os.Stdout, l)
    log.SetOutput(mw)
  } else {
    log.SetOutput(l)
  }
}
</code-block>

## Log Message Structure
<code-block lang="go" src="logger/message.go" collapsible="true"/>

## Log Level
<code-block lang="go" src="logger/level.go" collapsible="true"/>

## Logger
<code-block lang="go" src="logger/logger.go" collapsible="true"/>

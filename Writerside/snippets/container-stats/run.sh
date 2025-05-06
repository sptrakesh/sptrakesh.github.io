#!/bin/sh

. ~/.kshrc
PID_FILE='/tmp/container-statsd.pid'
LOG_FILE='/tmp/container-statsd.log'

export RUST_LOG=info

case "$1" in
  'start')
    /home/ec2-user/docker/container-statsd --node $HOST_NODE --questdb $PRODM --mode max --watchdog disabled >> $LOG_FILE 2>&1 &
    echo $! > $PID_FILE
    ;;
  'stop')
    kill `cat $PID_FILE`
    rm $PID_FILE
    ;;
  'log')
    tail -f $LOG_FILE
    ;;
  'ps')
    if [ -f $PID_FILE ]
    then
      ps -fp `cat $PID_FILE`
    else
      exit 1
    fi
    ;;
  *)
    echo "Usage: $0 <start|stop|log|ps>"
    exit 1
    ;;
esac

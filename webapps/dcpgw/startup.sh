#!/bin/bash

pid=`cat SRV_P.PID`

if [ "$pid" == "" ]; then
./mv_logs.sh
. ./setenv.sh

  PARAM="-Ddcpgw -Xmx256m -cp "$CLASSPATH" mobi.eyeline.dcpgw.Gateway"
  nohup $JAVA_HOME/bin/java $PARAM 1>logs/out 2>logs/err &
  echo Gateway successfully started.
  echo $!>SRV_P.PID
else
  echo Gateway already started. PID=$pid
fi
#./alert.sh start


#!/bin/bash

pid=`cat SRV_P.PID`

if [ "$pid" == "" ]; then
./mv_logs.sh
. ./setenv.sh

  PARAM="-Dsmpp_client_csl -Dfile.encoding=UTF-8 -Xmx32m -cp "$CLASSPATH" RegDeliveryReceiptsTest 100"
  nohup $JAVA_HOME/bin/java $PARAM 1>logs/out 2>logs/err &
  echo Smpp client successfully started.
  echo $!>SRV_P.PID
else
  echo Smpp client already started. PID=$pid
fi
#./alert.sh start
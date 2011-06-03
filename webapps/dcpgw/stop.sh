#!/bin/bash

pid=`cat SRV_P.PID`

if [ "$pid" == "" ]; then
  echo Gateway not started.
else
  echo Service PID=$pid
  `kill -3 $pid`
  echo Threads dumped.
  `kill -9 $pid`
  echo Gateway $pid stopped.
  rm SRV_P.PID
fi
#./alert.sh stop
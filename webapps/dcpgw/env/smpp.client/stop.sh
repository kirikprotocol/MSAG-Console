#!/bin/bash

pid=`cat SRV_P.PID`

if [ "$pid" == "" ]; then
  echo Weather reader not started.
else
  echo Service PID=$pid
  `kill -3 $pid`
  echo Threads dumped.
  `kill -9 $pid`
  echo Weather reader $pid stopped.
  rm SRV_P.PID
fi
#./alert.sh stop
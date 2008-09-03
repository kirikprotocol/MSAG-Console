#!/bin/sh

cd ..
pid=`cat MCAHDBTEST.PID`

if [ "$pid" == "" ]; then
  echo MCAHDB test not started.
else
  echo Service PID=$pid
  `kill -3 $pid`
  echo Threads dumped.
  `kill -9 $pid`
  echo Service $pid stopped.
  rm MCAHDBTEST.PID
fi

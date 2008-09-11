#!/bin/sh

cd ..
pid=`cat MCAHDB.PID`

if [ "$pid" == "" ]; then
  echo MCAHDB not started.
else
  echo Service PID=$pid
  `kill -3 $pid`
  echo Threads dumped.
fi

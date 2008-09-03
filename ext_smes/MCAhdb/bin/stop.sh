#!/bin/sh

cd ..
pid=`cat MCAHDB.PID`

if [ "$pid" == "" ]; then
  echo MCAHDB not started.
else
  echo Service PID=$pid
  `kill -3 $pid`
  echo Threads dumped.
  `kill $pid`
  echo Service $pid stopped.
  rm MCAHDB.PID
fi

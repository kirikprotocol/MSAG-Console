#!/bin/sh

cd ..
pid=`cat MCAHDB.PID`

if [ "$pid" == "" ]; then
  echo MCAHDB not started.
else
  echo Service PID=$pid
  `kill $pid`
  echo Service $pid stopped.
  rm MCAHDB.PID
fi

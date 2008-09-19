#!/bin/sh

cd ..
pid=`cat sponsored.PID`

if [ "$pid" == "" ]; then
  echo Sponsored not started.
else
  echo Service PID=$pid
  `kill $pid`
  echo Sponsored stopped.
  rm sponsored.PID
fi

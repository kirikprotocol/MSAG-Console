#!/bin/sh

date_=`date`

echo $date_: STOP. PWD=$PWD USER=$USER CLIENT=$SSH_CLIENT>>log

cd ..

pid=`cat SMSQUIZ.PID`

if [ "$pid" == "" ]; then
  echo Service not started.
else
  echo Service PID=$pid
  `kill -3 $pid`
  echo Threads dumped.
  `kill -9 $pid`
  echo Service $pid stopped.
  rm SMSQUIZ.PID 2>&1
fi

#!/bin/sh

d=`date +%Y%m%d%H%M%S`

if [ -d "logs.old/$d" ]; then
  echo Could not copy logs. Logs directory $d already exists.
else
  mkdir -p logs.old/$d 2>&1
  mv logs/* logs.old/$d/ 2>&1
  echo Logs copied.
fi

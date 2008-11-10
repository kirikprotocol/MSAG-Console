#!/bin/bash

SPONS_PLATFORM=linux32

cd ..
pid=`cat sponsored.PID`

if [ "$pid" == "" ]; then
  LD_LIBRARY_PATH=lib/platform/$SPONS_PLATFORM
  cls=conf
  for i in lib/*.jar ; do cls=$cls:$i ; done
  java -Dsmsc_sponsored -cp $cls -Xmx128M -Dfile.encoding=windows-1251 -Djava.library.path=$LD_LIBRARY_PATH com.eyeline.sponsored.Sme $@ 1>logs/sponsored.out 2>logs/sponsored.err &
  echo Sponsored successfully started.
  echo $!>./sponsored.PID
else
  echo Sponsored already started. PID=$pid
fi
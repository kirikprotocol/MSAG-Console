#!/bin/bash

pid=`cat MCAHDB.PID`
if [ "$pid" == "" ]; then
cd ..

cls=conf
for i in lib/*.jar ; do cls=$cls:$i ; done
java -cp $cls -Darrivedok_scheduller mobi.eyeline.mcahdb.MCAHDB $@ 1>logs/mcahdb_out.log 2>logs/mcahdb_err.log &
echo MCAHDB successfully started.
echo $!>./MCAHDB.PID
else
echo MCAHDB already started. PID=$pid
fi



#!/bin/bash

pid=`cat MCAHDBTEST.PID`
if [ "$pid" == "" ]; then
cd ..

cls=conf
for i in lib/*.jar ; do cls=$cls:$i ; done
java -cp $cls -Darrivedok_scheduller mobi.eyeline.mcahdb.MCAHDBTest $@ 1>logs/mcahdb_test_out.log 2>logs/mcahdb_test_err.log &
echo MCAHDB successfully started.
echo $!>./MCAHDBTEST.PID
else
echo MCAHDB already started. PID=$pid
fi
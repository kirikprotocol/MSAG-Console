#!/bin/bash

./mv_logs.sh
. ./setenv.sh

PARAM="-Ddcpgw_st3 -Xmx32m -cp "$CLASSPATH" mobi.eyeline.dcpgw.tests.stress.Test2"
echo Start ...
$JAVA_HOME/bin/java $PARAM config1.properties config3.properties
echo Finish!
#!/bin/bash

./mv_logs.sh
. ./setenv.sh

PARAM="-Ddcpgw_st1 -Xmx32m -cp "$CLASSPATH" mobi.eyeline.dcpgw.tests.stress.Test"
echo Start ...
$JAVA_HOME/bin/java $PARAM
echo Finish!

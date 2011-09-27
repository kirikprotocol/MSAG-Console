#!/bin/sh

. ./setenv.sh

PARAM="-Ddcpgw_st -Xmx32m -cp "$CLASSPATH" org.junit.runner.JUnitCore mobi.eyeline.dcpgw.tests.stress.StressTest"
echo Start ...
$JAVA_HOME/bin/java $PARAM
echo Finish!

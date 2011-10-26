#!/bin/bash

./mv_logs.sh
. ./setenv.sh

PARAM="-Dsmpp_client_resend_test_2 -Dfile.encoding=UTF-8 -Xmx32m -cp "$CLASSPATH" ResendTest2 100"
$JAVA_HOME/bin/java $PARAM

echo Finish!
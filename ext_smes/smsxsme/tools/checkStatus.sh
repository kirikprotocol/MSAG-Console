#!/bin/sh

. ./setenv.sh

java -cp $SMSX_CLASSPATH $SMSX_SSL_OPTS ru.sibinco.smsx.tools.CheckStatus $SMSX_SENDER_URL $@
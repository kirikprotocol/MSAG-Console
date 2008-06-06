#!/bin/sh

. ./setenv.sh

java -cp $SMSX_CLASSPATH $SMSX_SSL_OPTS ru.sibinco.smsx.SmsXSenderLoadTest $SMSX_SENDER_URL $@
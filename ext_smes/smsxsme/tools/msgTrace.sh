#!/bin/sh

. ./setenv.sh

java -cp $SMSX_CLASSPATH com.eyeline.sme.handler.SmsTrace $SMSX_CONF/services.xml
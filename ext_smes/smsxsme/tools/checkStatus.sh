#!/bin/sh

. ./classpath.sh

java -cp $CLASSPATH ru.sibinco.smsx.tools.CheckStatus "http://192.168.1.194:8080/smsx/services/SmsXSender" $1
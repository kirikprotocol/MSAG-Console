#!/bin/sh

. ./classpath.sh
export OPTS="-Djavax.net.ssl.keyStore=../conf/truststore.jks -Djavax.net.ssl.keyStorePassword=laefeeza -Djavax.net.ssl.trustStore=../conf/keystore.jks -Djavax.net.ssl.trustStorePassword=laefeeza -DtrustAnchors=true" 

java -cp $CLASSPATH $OPTS ru.sibinco.smsx.tools.SendSMSMsg  "https://192.168.1.194:8443/smsx/services/SmsXSender" $1 $2 $3 $4
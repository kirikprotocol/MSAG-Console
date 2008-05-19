#!/bin/sh

# Set classpath
SMSX_CLASSPATH_HOME=../webapps/smsx/WEB-INF/lib
SMSX_CONF=../conf
SMSX_CLASSPATH=$SMSX_CONF
for i in SMSX_CLASSPATH_HOME/*.jar ; do SMSX_CLASSPATH=$cls:$i ; done

# Set SSL connection options
SMSX_SSL_OPTS="-Djavax.net.ssl.keyStore=$SMSX_CONF/truststore.jks -Djavax.net.ssl.keyStorePassword=laefeeza -Djavax.net.ssl.trustStore=$SMSX_CONF/keystore.jks -Djavax.net.ssl.trustStorePassword=laefeeza -DtrustAnchors=true"

# Set black list url
SMSX_BLACK_LIST_URL="http://192.168.1.194:8080/smsx/services/blacklistSoap"

# Set Smsx sender url
SMSX_SENDER_URL="http://192.168.1.194:8080/smsx/services/SmsXSender"
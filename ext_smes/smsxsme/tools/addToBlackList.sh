#!/bin/sh

. ./classpath.sh

java -cp $CLASSPATH ru.sibinco.smsx.tools.AddToBlackList  "http://192.168.1.194:8080/smsx/services/blacklistSoap" $1
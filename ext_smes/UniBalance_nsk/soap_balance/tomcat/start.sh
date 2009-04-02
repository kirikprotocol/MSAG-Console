#!/bin/bash

./remove_logs.sh
export CATALINA_HOME=$HOME/software/tomcat6
export CATALINA_BASE=.
export JAVA_HOME=/opt/sun-jdk-1.6.0.06
#export JAVA_OPTS='-Xdebug -Xrunjdwp:transport=dt_socket,server=y,suspend=y,address=5005'
#export JAVA_OPTS='-verbose:gc'
export JAVA_OPTS='-Xmx256M -Xms128M'
$CATALINA_HOME/bin/catalina.sh start

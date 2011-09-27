#export JAVA_HOME=/opt/java/j2sdk1.4.2_19

CLASSPATH=.:$JAVA_HOME/jre/lib/rt.jar:conf

CLASSPATH=$CLASSPATH:lib/junit-4.9.jar
CLASSPATH=$CLASSPATH:lib/smppapi.jar
CLASSPATH=$CLASSPATH:lib/smppcharsets.jar
CLASSPATH=$CLASSPATH:lib/log4j-1.2.16.jar
CLASSPATH=$CLASSPATH:lib/utils.jar

#echo $CLASSPATH
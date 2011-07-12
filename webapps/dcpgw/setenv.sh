#export JAVA_HOME=/opt/java/j2sdk1.4.2_19

CLASSPATH=.:$JAVA_HOME/jre/lib/rt.jar:config

CLASSPATH=$CLASSPATH:lib/dcpgw.jar
CLASSPATH=$CLASSPATH:lib/smppapi.jar
CLASSPATH=$CLASSPATH:lib/smppcharsets.jar
CLASSPATH=$CLASSPATH:lib/utils.jar
CLASSPATH=$CLASSPATH:lib/log4j-1.2.16.jar
CLASSPATH=$CLASSPATH:lib/jdom.jar
CLASSPATH=$CLASSPATH:lib/xercesImpl.jar
CLASSPATH=$CLASSPATH:lib/org.eclipse.wst.xml.xpath2.processor_1.1.0.jar
CLASSPATH=$CLASSPATH:lib/protogenfw.jar
CLASSPATH=$CLASSPATH:lib/apm_utils.jar
CLASSPATH=$CLASSPATH:lib/commons-io-2.0.1.jar
CLASSPATH=$CLASSPATH:lib/smppsme.jar

#echo $CLASSPATH
CLASSPATH=lib/activation.jar
CLASSPATH=$CLASSPATH:lib/axis-ant.jar
CLASSPATH=$CLASSPATH:lib/axis.jar
CLASSPATH=$CLASSPATH:lib/commons-discovery-0.2.jar
CLASSPATH=$CLASSPATH:lib/commons-logging-1.0.4.jar
CLASSPATH=$CLASSPATH:lib/jaxrpc.jar
CLASSPATH=$CLASSPATH:lib/log4j-1.2.8.jar
CLASSPATH=$CLASSPATH:lib/log4j.properties
CLASSPATH=$CLASSPATH:lib/saaj.jar
CLASSPATH=$CLASSPATH:lib/saaj.jar
CLASSPATH=$CLASSPATH:lib/wsdl4j-1.5.1.jar

java -cp $CLASSPATH org.apache.axis.wsdl.WSDL2Java -s --skeletonDeploy true $1


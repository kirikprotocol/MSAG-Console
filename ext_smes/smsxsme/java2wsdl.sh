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

java -cp $CLASSPATH:test/classes org.apache.axis.wsdl.Java2WSDL -T 1.2 -y WRAPPED -o test.wsdl -l "http://localhost:8080/webservice4/services/SmsXSender" -n "http://sibinco.ru/smsXSend" -p"ru.sibinco.smsxSend" "sd" ru.sibinco.smsXSend.SmsXSender


CLASSPATH_HOME=webapps/smsx/WEB-INF/lib
CLASSPATH=$CLASSPATH_HOME/activation.jar
CLASSPATH=$CLASSPATH:$CLASSPATH_HOME/axis-ant.jar
CLASSPATH=$CLASSPATH:$CLASSPATH_HOME/axis.jar
CLASSPATH=$CLASSPATH:$CLASSPATH_HOME/commons-discovery-0.2.jar
CLASSPATH=$CLASSPATH:$CLASSPATH_HOME/commons-logging-1.0.4.jar
CLASSPATH=$CLASSPATH:$CLASSPATH_HOME/jaxrpc.jar
CLASSPATH=$CLASSPATH:$CLASSPATH_HOME/log4j-1.2.8.jar
CLASSPATH=$CLASSPATH:$CLASSPATH_HOME/log4j.properties
CLASSPATH=$CLASSPATH:$CLASSPATH_HOME/saaj.jar
CLASSPATH=$CLASSPATH:$CLASSPATH_HOME/saaj.jar
CLASSPATH=$CLASSPATH:$CLASSPATH_HOME/wsdl4j-1.5.1.jar
CLASSPATH=$CLASSPATH:$CLASSPATH_HOME/smsx.jar

java -cp $CLASSPATH org.apache.axis.client.AdminClient -l http://localhost:8080/smsx/servlet/AxisServlet soap/senderdeploy.wsdd
java -cp $CLASSPATH org.apache.axis.client.AdminClient -l http://localhost:8080/smsx/servlet/AxisServlet soap/blacklistdeploy.wsdd
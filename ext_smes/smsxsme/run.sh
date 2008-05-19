export CATALINA_HOME=.
export CATALINA_BASE=.
#export JAVA_OPTS=-Dtest.mode
export JAVA_OPTS="-Dtest.mode -Dcom.sun.management.jmxremote.port=10999 -Dcom.sun.management.jmxremote.authenticate=false -Dcom.sun.management.jmxremote.ssl=false"
./bin/catalina.sh start 1>logs/tomcat.out 2>logs/tomcat.err
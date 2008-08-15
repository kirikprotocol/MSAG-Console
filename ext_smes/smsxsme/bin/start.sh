export CATALINA_HOME=.
export CATALINA_BASE=.

LD_LIBRARY_PATH=common/lib/platform/linux32

export JAVA_OPTS="-Dtest.mode -Djava.library.path=$LD_LIBRARY_PATH"
#export JAVA_OPTS="-Djava.library.path=$LD_LIBRARY_PATH -Dcom.sun.management.jmxremote.port=10999 -Dcom.sun.management.jmxremote.authenticate=false -Dcom.sun.management.jmxremote.ssl=false"

./bin/catalina.sh start 1>logs/tomcat.out 2>logs/tomcat.err
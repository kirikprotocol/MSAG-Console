export CATALINA_HOME=.
export CATALINA_BASE=.

LD_LIBRARY_PATH=common/lib/platform/linux32
export JAVA_OPTS="-Dtest.mode -Djava.library.path=$LD_LIBRARY_PATH"
#export JAVA_OPTS="-Djava.library.path=$LD_LIBRARY_PATH -Dcom.sun.management.jmxremote.port=10999 -Dcom.sun.management.jmxremote.authenticate=false -Dcom.sun.management.jmxremote.ssl=false"

pid=`cat smsx.pid`
if [ "$pid" == "" ]; then
  ./bin/catalina.sh run 1>logs/tomcat.out 2>logs/tomcat.err &
  echo "SMSX succesfully started"
  echo $! > smsx.pid
else
  echo "SMSX already started. pid=$pid"
fi
export CATALINA_HOME=$HOME/software/tomcat6
export CATALINA_BASE=.
export JAVA_HOME=/opt/sun-jdk-1.6.0.06
$CATALINA_HOME/bin/catalina.sh stop
pids=`ps aux|grep tomcat|grep java|awk '{print $2}'`
WC=`echo $pids | wc -w`
[ $WC -gt 0 ] && sleep 3 && echo "$WC processes killed" && kill -3 $pids && kill $pids
export JAVA_HOME=@JAVA_HOME@
export JAVA_OPTS="@JAVA_OPTS@"
export CATALINA_HOME=tomcat
export CATALINA_BASE=.
$CATALINA_HOME/bin/catalina.sh $1 1>@logs.dir@/tomcat.out 2>&1
if [ "$2" != "" ] ; then 
  rm -f $2
fi

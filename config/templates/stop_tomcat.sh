if [ ! -e "tomcat.pid" ] ; then
  echo "Tomcat is not started"
  exit 1
fi
tomcat/tomcat_runner.sh stop
echo "Waiting tomcat to stop"
while [ -e "tomcat.pid" ] ; 
do
  echo -n "." ;
  usleep 1 ;
done


if [ ! -e "daemon.pid" ] ; then
  echo "Daemon is not started"
  exit 1
fi
kill `cat daemon.pid`
echo "Waiting daemon to stop"
while [ -e "daemon.pid" ] ; 
do
  echo -n "." ;
  usleep 1000000 ;
done


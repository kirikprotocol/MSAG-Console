pid=`cat smsx.pid`
if [ "$pid" == "" ]; then
  echo "SMSX is not started."
else
  ./bin/catalina.sh stop
   echo Service PID=$pid
  `kill -9 $pid`
  echo Service $pid stopped.
  rm smsx.pid
fi
bin/smsc_ssdaemon conf/daemon.xml 1>logs/daemon.out 2>&1 &
PID=$!
export PID
trap "kill -s SIGTERM $PID;wait $PID; exit" SIGTERM
wait $PID
rm -f $1

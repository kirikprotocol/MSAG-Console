#! /bin/bash

echo $$ >pid

while true;
do
./smsc 2>err.log &
PID=$!
export PID
trap "kill -2 $PID" SIGINT
wait $PID
if [ -f core ]; then
  DATE=`perl -e '($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);printf("%04d%02d%02d%02d%02d%02d",$year+1900,$mon,$mday,$hour,$min,$sec)'`
  mkdir "$DATE"
  mv core "$DATE"
  mv err.log "$DATE"
  mv smsc.log "$DATE"
  mv smsc "$DATA"
fi
done

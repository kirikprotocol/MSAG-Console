#! /bin/bash
export LC_ALL=ru_RU.ANSI1251            
export NLS_LANG=RUSSIAN_CIS.CL8MSWIN1251

echo $$ >pid
if [ -f logs/smsc.err ]; then
  mv logs/smsc.err logs/smsc.err.old
fi
while true;
do
./bin/smsc 1>/dev/null 2>>logs/smsc.err&
PID=$!
export PID
trap "kill -2 $PID;wait $PID" SIGINT
wait $PID
if [ -f core ]; then
  DATE=`perl -e '($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);printf("%04d%02d%02d%02d%02d%02d",$year+1900,$mon+1,$mday,$hour,$min,$sec)'`
  mkdir "$DATE"
  mv core "$DATE"
  mv logs/smsc.err "$DATE"
  if [ -f logs/smsc.err.old ]; then
    mv logs/smsc.err.old "$DATE"
  fi
  mv smsc.log "$DATE"
  cp -p ./bin/smsc "$DATE"
fi
done

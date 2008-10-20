#!/bin/sh


cd ..

platform="linux32"
cls="conf/"
for i in lib/*.jar ; do cls=$cls:$i ; done

PARAM="-Xmx128m -cp $cls -Dfile.encoding=windows-1251 -Djava.library.path=lib/platform/$platform  mobi.eyeline.smsquiz.Main"

pid=`cat SMSQUIZ.PID`

if [ "$pid" == "" ]; then

    $JAVA_HOME/bin/java $PARAM  &
    echo $!>SMSQUIZ.PID
    echo Service started
  else
    echo Service already started. PID=$pid
fi

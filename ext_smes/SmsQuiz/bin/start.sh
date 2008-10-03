#!/bin/sh
if [[ $# < 1 ]]; then
  echo "Destination file name expected"
  exit 1
fi

cd ..

cls="conf/"
for i in lib/*.jar ; do cls=$cls:$i ; done

PARAM="-Dtest_$1_$2 -Xmx128m -cp $cls -Dfile.encoding=windows-1251 mobi.eyeline.smsquiz.script.SubscribedAbDownloader"

echo "Downloading..."
$JAVA_HOME/bin/java $PARAM $@

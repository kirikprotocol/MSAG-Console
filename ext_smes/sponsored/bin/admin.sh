#!/bin/bash

if [[ $# < 1 ]]; then
  echo "Invalid distribution name. Print 'subscrexport.sh --help' for help."
else
  if [ "$1" == "--help" ]; then
    echo ""
    echo "Usage: subscrexport.sh [OPTS] <distribution name>"
    echo "Options:"
    echo "  -d Export subscriptions for specified date"
    echo "  -r Group subscriptions by routes"
cd ..
for i in lib/*.jar ; do cls=$cls:$i ; done
java -cp $cls -Xmx128M -DsubscrExport -Dfile.encoding=windows-1251 com.eyeline.sponsored.subscription.admin.SubscriptionsExport $@
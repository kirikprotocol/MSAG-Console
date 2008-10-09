#!/bin/bash

MCA_URL="http://phoenix:8008/mca/services/MissedCalls"

if [[ $1 == "get" ]]; then
    SCRIPT_NAME="getevents.sh"
else
    echo -e "Usage: \n get\n"
    exit 1
fi

shift

SCRIPT_NAME="./mca/$SCRIPT_NAME"

./sendsoap.sh $MCA_URL $SCRIPT_NAME $@
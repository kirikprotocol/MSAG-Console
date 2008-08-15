#!/bin/bash

BLACK_LIST_URL="http://localhost:8080/smsx/services/blacklistSoap"

if [[ $1 == "add" ]]; then
    SCRIPT_NAME="add.sh"
elif [[ $1 == "remove" ]]; then
    SCRIPT_NAME="remove.sh"
else
    echo -e "Usage: \n add\n remove\n"
    exit 1
fi

shift

SCRIPT_NAME="./blacklist/$SCRIPT_NAME"

./sendsoap.sh $BLACK_LIST_URL $SCRIPT_NAME $@
#!/bin/bash

GROUP_SEND_URL="http://localhost:8080/smsx/services/GroupSend"

if [[ $1 == "send" ]]; then
    SCRIPT_NAME="groupsend.sh"
elif [[ $1 == "check" ]]; then
    SCRIPT_NAME="checkstatus.sh"
else
    echo -e "Usage: \n send\n check\n"
    exit 1
fi

shift

SCRIPT_NAME="./groupsend/$SCRIPT_NAME"

./sendsoap.sh $GROUP_SEND_URL $SCRIPT_NAME $@
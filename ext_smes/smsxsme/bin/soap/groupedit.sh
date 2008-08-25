#!/bin/bash

GROUP_EDIT_URL="http://phoenix:7802/smsx/services/GroupEdit"

if [[ $1 == "add" ]]; then
    SCRIPT_NAME="groupadd.sh"
elif [[ $1 == "list" ]]; then
    SCRIPT_NAME="grouplist.sh"
elif [[ $1 == "remove" ]]; then
    SCRIPT_NAME="groupremove.sh"
elif [[ $1 == "rename" ]]; then
    SCRIPT_NAME="grouprename.sh"
elif [[ $1 == "view" ]]; then
    SCRIPT_NAME="groupview.sh"
elif [[ $1 == "removerecipient" ]]; then
    SCRIPT_NAME="recipientremove.sh"
elif [[ $1 == "addrecipient" ]]; then
    SCRIPT_NAME="recipientadd.sh"
else
    echo -e "Usage: \n add\n list\n remove\n rename\n view\n removerecipient\n addrecipient\n"
    exit 1
fi

shift

SCRIPT_NAME="./groupedit/$SCRIPT_NAME"

./sendsoap.sh $GROUP_EDIT_URL $SCRIPT_NAME $@
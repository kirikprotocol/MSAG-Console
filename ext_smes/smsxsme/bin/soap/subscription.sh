#!/bin/bash

SUBSCRIPTION_URL="http://localhost:8080/smsx/services/SmsXSubscription"

if [[ $1 == "check" ]]; then
    SCRIPT_NAME="check.sh"
else
    echo -e "Usage: \n check\n"
    exit 1
fi

shift

SCRIPT_NAME="./subscription/$SCRIPT_NAME"

./sendsoap.sh $SUBSCRIPTION_URL $SCRIPT_NAME $@
#!/bin/bash

SMSXSENDER_URL="https://localhost:8443/smsx/services/SmsXSender"

if [[ $1 == "secret" ]]; then
    SCRIPT_NAME="sendsecret.sh"
elif [[ $1 == "calendar" ]]; then
    SCRIPT_NAME="sendcalendar.sh"
elif [[ $1 == "sms" ]]; then
    SCRIPT_NAME="sendsms.sh"
elif [[ $1 == "smspaid" ]]; then
    SCRIPT_NAME="sendpaidsms.sh"
elif [[ $1 == "smssys" ]]; then
    SCRIPT_NAME="sendsyssms.sh"
elif [[ $1 == "calendarpaid" ]]; then
    SCRIPT_NAME="sendpaidcalendar.sh"
elif [[ $1 == "secretpaid" ]]; then
    SCRIPT_NAME="sendpaidsecret.sh"
elif [[ $1 == "check" ]]; then
    SCRIPT_NAME="checkstatus.sh"
else
    echo -e "Usage: \n sms\n secret\n calendar\n smspaid\n secretpaid\n calendarpaid\n smssys\n check\n"
    exit 1
fi

shift

SCRIPT_NAME="./smsxsender/$SCRIPT_NAME"

./sendsoap.sh $SMSXSENDER_URL $SCRIPT_NAME $@
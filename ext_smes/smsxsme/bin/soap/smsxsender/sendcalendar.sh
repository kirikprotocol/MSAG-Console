#!/bin/bash

if [[ $# < 5 ]]; then
    echo "caledar <msisdn> <message> <express> <advertising> <delay in seconds>"
    exit 1
fi

x=`date +%s`
x=($x+$5)*1000

echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<soapenv:Envelope xmlns:soapenv=\"http://www.w3.org/2003/05/soap-envelope\"
                  xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"
                  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">
  <soapenv:Body>
    <sendSms xmlns=\"http://sibinco.ru/smsXSend\">
      <MSISDN>$1</MSISDN>
      <Message>$2</Message>
      <SMSXExpress>$3</SMSXExpress>
      <SMSXSecret>false</SMSXSecret>
      <SMSXCalendar>true</SMSXCalendar>
      <SMSXCalendarTimeUTC>$x</SMSXCalendarTimeUTC>
      <SMSXAdvertising>$4</SMSXAdvertising>
    </sendSms>
  </soapenv:Body>
</soapenv:Envelope>"
#!/bin/bash

if [[ $# < 4 ]]; then
    echo "secret <msisdn> <message> <express> <advertising>"
    exit 1
fi

echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<soapenv:Envelope xmlns:soapenv=\"http://www.w3.org/2003/05/soap-envelope\" 
                  xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" 
                  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">
  <soapenv:Body>
    <sendSms xmlns=\"http://sibinco.ru/smsXSend\">
      <MSISDN>$1</MSISDN>
      <Message>$2</Message>
      <SMSXExpress>$3</SMSXExpress>
      <SMSXSecret>true</SMSXSecret>
      <SMSXCalendar>false</SMSXCalendar>
      <SMSXCalendarTimeUTC>0</SMSXCalendarTimeUTC>
      <SMSXAdvertising>$4</SMSXAdvertising>
    </sendSms>
  </soapenv:Body>
</soapenv:Envelope>"
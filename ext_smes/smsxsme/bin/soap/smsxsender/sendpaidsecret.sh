#!/bin/bash

if [[ $# < 4 ]]; then
    echo "secretpaid <oa> <da> <message> <express>"
    exit 1
fi

echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<soapenv:Envelope xmlns:soapenv=\"http://www.w3.org/2003/05/soap-envelope\"
                  xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"
                  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">
  <soapenv:Body>
    <sendPaidSms xmlns=\"http://sibinco.ru/smsXSend\">
      <SourceAddress>$1</SourceAddress>
      <DestinationAddress>$2</DestinationAddress>
      <Message>$3</Message>
      <SMSXExpress>$4</SMSXExpress>
      <SMSXSecret>true</SMSXSecret>
      <SMSXCalendar>false</SMSXCalendar>
      <SMSXCalendarTimeUTC>0</SMSXCalendarTimeUTC>
    </sendPaidSms>
  </soapenv:Body>
</soapenv:Envelope>"
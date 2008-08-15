#!/bin/bash

if [[ $# < 1 ]]; then
    echo "check <msisdn>"
    exit 1
fi

echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"
 xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">
  <soapenv:Body>
    <checkSubscription xmlns=\"http://sibinco.ru/smsxsubscription\">
       <msisdn>$1</msisdn>
    </checkSubscription>
  </soapenv:Body>
</soapenv:Envelope>"
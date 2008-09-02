#!/bin/bash

if [[ $# < 1 ]]; then
    echo "get <msisdn>"
    exit 1
fi

echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>
<soapenv:Envelope xmlns:soapenv=\"http://www.w3.org/2003/05/soap-envelope\">
  <soapenv:Body>
    <ns1:GetMissedCalls xmlns:ns1=\"http://tempuri.org/\">
      <ns1:phoneNumber>$1</ns1:phoneNumber>
    </ns1:GetMissedCalls>
  </soapenv:Body>
</soapenv:Envelope>"



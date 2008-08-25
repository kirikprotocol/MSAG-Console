#!/bin/bash

if [[ $# < 3 ]]; then
    echo "smssys <oa> <da> <message>"
    exit 1
fi

echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<soapenv:Envelope xmlns:soapenv=\"http://www.w3.org/2003/05/soap-envelope\"
                  xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"
                  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">
  <soapenv:Body>
    <sendSysSms xmlns=\"http://sibinco.ru/smsXSend\">
      <SourceAddress>$1</SourceAddress>
      <DestinationAddress>$2</DestinationAddress>
      <Message>$3</Message>
    </sendSysSms>
  </soapenv:Body>
</soapenv:Envelope>"
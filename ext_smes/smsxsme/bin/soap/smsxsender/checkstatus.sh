#!/bin/bash
              
if [[ $# < 1 ]]; then
    echo "check <msgid>"
    exit 1
fi

echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<soapenv:Envelope xmlns:soapenv=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"
xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">
  <soapenv:Body>
    <checkStatus xmlns=\"http://sibinco.ru/smsXSend\">
      <SMSXIdMessage>$1</SMSXIdMessage>
    </checkStatus>
  </soapenv:Body>
</soapenv:Envelope>"
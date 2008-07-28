#!/bin/bash

if [[ $# < 1 ]]; then
    echo "checkstatus <msgid>"
    exit 1
fi

echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"
 xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">
  <soapenv:Body>
      <checkStatus xmlns=\"http://sibinco.ru/groupsend\">
            <msgId>$1</msgId>
                </checkStatus>
                  </soapenv:Body>
                  </soapenv:Envelope>"
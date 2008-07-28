#!/bin/bash

if [[ $# < 4 ]]; then
    echo "groupsend <groupname> <owner> <message> <express>"
    exit 1
fi

echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" 
xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">
  <soapenv:Body>
      <sendSms xmlns=\"http://sibinco.ru/groupsend\">
            <groupName>$1</groupName>
                  <owner>$2</owner>
                        <message>$3</message>
                              <express>$4</express>
                                  </sendSms>
                                    </soapenv:Body>
                                    </soapenv:Envelope>"
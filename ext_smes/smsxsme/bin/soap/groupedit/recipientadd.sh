#!/bin/bash

if [[ $# < 3 ]]; then
    echo "addrecipient <groupname> <owner> <recipient>"
    exit 1
fi

echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"
xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">
<soapenv:Body>
<addMember xmlns=\"http://sibinco.ru/groupedit\">
<groupName>$1</groupName>
<owner>$2</owner>
<member>$3</member>
</addMember>
</soapenv:Body>
</soapenv:Envelope>"
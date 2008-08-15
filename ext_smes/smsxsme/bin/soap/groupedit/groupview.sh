#!/bin/bash

if [[ $# < 2 ]]; then
    echo "view <groupname> <owner>"
    exit 1
fi

echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"
xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">
<soapenv:Body>
<groupInfo xmlns=\"http://sibinco.ru/groupedit\">
<groupName>$1</groupName>
<owner>$2</owner>
</groupInfo>
</soapenv:Body>
</soapenv:Envelope>"
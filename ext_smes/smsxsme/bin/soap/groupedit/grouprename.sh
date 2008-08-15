#!/bin/bash

if [[ $# < 3 ]]; then
    echo "rename <groupname> <owner> <newname>"
    exit 1
fi

echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"
xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">
<soapenv:Body>
<renameGroup xmlns=\"http://sibinco.ru/groupedit\">
<groupName>$1</groupName>
<owner>$2</owner>
<newGroupName>$3</newGroupName>
</groupInfo>
</soapenv:Body>
</soapenv:Envelope>"
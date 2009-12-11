#!/bin/bash

if [[ $# < 1 ]]; then
    echo "check <msisdn>"
    exit 1
fi

echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>
<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"
xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">
  <soap:Body>
    <Check xmlns=\"http://mts.qmobile.ru/api/\">
      <msisdn>$1</msisdn>
    </Check>
  </soap:Body>
</soap:Envelope>"
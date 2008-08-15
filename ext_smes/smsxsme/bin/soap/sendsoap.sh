#!/bin/bash

URL=$1
SCRIPT=$2

shift
shift

`$SCRIPT $@ > req.txt`

if [[ $? -eq 0 ]]; then
    echo -e "\nREQUEST:\n"
    cat req.txt

    wget --post-file=req.txt -v -O resp.txt --no-check-certificate $URL

    echo -e "\nRESPONSE:\n"
    cat resp.txt
    echo -e "\n"

    rm resp.txt
else
    cat req.txt
fi

rm req.txt


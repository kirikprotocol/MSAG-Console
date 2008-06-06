#!/bin/sh

. ./setenv.sh

java -cp $SMSX_CLASSPATH ru.sibinco.smsx.AddToBlackList $SMSX_BLACK_LIST_URL $@
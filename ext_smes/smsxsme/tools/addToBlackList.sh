#!/bin/sh

. ./setenv.sh

java -cp $SMSX_CLASSPATH ru.sibinco.smsx.tools.AddToBlackList $SMSX_BLACK_LIST_URL $@
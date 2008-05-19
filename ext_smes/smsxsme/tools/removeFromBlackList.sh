#!/bin/sh

. ./setenv.sh

java -cp $SMSX_CLASSPATH ru.sibinco.smsx.tools.RemoveFromBlackList $SMSX_BLACK_LIST_URL $@
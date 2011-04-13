#!/bin/sh

PARAM="-Xmx128M -Dsmsx-stats-script"

java $PARAM -jar smsx_stat_script.jar ../../ 20110207 ../../conf ../../output $@

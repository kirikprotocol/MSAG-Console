#!/bin/sh
#
# Copyright (c) 1997 Ericsson Software Technology AB, Sweden.
# All rights reserved.
# 
SS7_HOME=/opt/EINss7166/
export SS7_HOME

SS7_CP_CONFIGFILE=/opt/EINss7166/bin/cp.cnf
export SS7_CP_CONFIGFILE

XENVIRONMENT=/opt/EINss7166/bin/ss7mgr.res
export XENVIRONMENT

DISPLAY=smsc.novosoft.ru:2
export DISPLAY

case "$1" in
'start')
    echo "Starting Log daemon process..."
    cd /opt/EINss7166/bin/
    ./EinSS7LogD &
    echo "Starting FrontEnd process.."
    cd /opt/EINss7129/bin/
    ./ife FEIF01 autostart ife01.cnf -t &
    echo "Starting BackEnd process.."
    cd /opt/EINss7166/bin/
    ./ss7 -t &
    sleep 2
    ./ss7run ss7.cnf ETSIMAP TCAP SCCP MTPL3 MTPL2
    echo "OK"
        ;;
'stop')
        pidlist=`/usr/bin/ps -ef`
        PID=`echo "$pidlist" | grep "./ss7" | awk '{print $2}'`
        PID1=`echo "$pidlist" | grep "./ife FEIF01 autostart" | awk '{print $2}'`
	PID2=`echo "$pidlist" | grep "./EinSS7LogD" | awk '{print $2}'`
    if [ ! -z "$PID" ]
        then
                echo "Stopping BackEnd process "
        /usr/bin/kill -TERM ${PID}
        else
                echo "***ERROR: The BackEnd process is not running"
    fi
    if [ ! -z "$PID1" ]
        then
               echo "Stopping FrontEnd process "
         /usr/bin/kill -TERM ${PID1}
          else
             echo "***ERROR: The FrontEnd process is not running"
    fi
    if [ ! -z "$PID2" ]
	then
	    echo "Stopping Log daemon process "
	    /usr/bin/kill -TERM ${PID2}
	else
	    echo "***ERROR: Log daemon is not running"
    fi

        ;;
'mgr')
    echo "Starting SS7 manager process"
    cd /opt/EINss7166/bin/
    ./ss7mgr &
	;;
*)
        echo "Usage: $0 { start | stop | mgr}"
        ;;
esac


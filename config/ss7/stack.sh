#!/bin/sh
#
# Copyright (c) 2003 Sibinco, Russia.
# All rights reserved.
# 
# nohup /opt/EINss7166/bin/stack.sh start >> /opt/EINss7166/bin/stack.log 2>&1 &

SS7_HOME=/opt/EINss7166/
export SS7_HOME
SS7_CP_CONFIGFILE=/opt/EINss7166/bin/cp.cnf
export SS7_CP_CONFIGFILE
XENVIRONMENT=/opt/EINss7166/bin/ss7mgr.res
export XENVIRONMENT
DISPLAY=smsc.novosoft.ru:2
export DISPLAY
installdir="${SS7_HOME}bin/"
cd $installdir
curwd=`pwd`
scriptname="stack.sh"

bess="./ss7"
be="cd /opt/EINss7166/bin/; ${bess} -t &"
logdss="./EinSS7LogD"
logd="cd /opt/EINss7166/bin; ${logdss} &"
ifess="./ife"
ife="cd /opt/EINss7129/bin/; ${ifess} FEIF01 autostart ife01.cnf -t &"
runcmd="cd /opt/EINss7166/bin/; ./ss7run ss7.cnf ETSIMAP TCAP SCCP MTPL3 MTPL2"


startall ()
{
    eval $logd
    cd $curwd
    logdpid=`ps -ef | grep "$logdss" | grep -v grep | awk '{print $2}'`
    echo "logd:$logdpid"
    if [ -z "$logdpid" ]
    then
	stopall
	return 13
    fi

    eval $ife
    cd $curwd
    ifepid=`ps -ef | grep "$ifess" | grep -v grep | awk '{print $2}'`
    echo "ife:$ifepid"
    #IFE CHECKING    
    if [ -z "$ifepid" ]
    then
	stopall
	return 13
    fi
    eval $be
    cd $curwd
    sleep 5
    bepid=`ps -ef | grep "$bess" | grep -v grep | awk '{print $2}'`
    echo "ss7:$bepid"
    #BE CHECKING    
    if [ -z "$bepid" ]
    then
	stopall
	return 13
    fi
    #echo "try to: $runcmd"
    eval $runcmd
    runstatus=$?
    cd $curwd
    #echo "runstatus=$runstatus"
    if [ $runstatus -ne 0 ]
    then
	stopall
	return 13
    fi
    return 0
}
stopall ()
{
    bepid=`ps -ef | grep "$bess" | grep -v grep | awk '{print $2}'`
    if [ ! -z "$bepid" ]
    then
        kill -9 $bepid
    fi
    
    ifepid=`ps -ef | grep "$ifess" | grep -v grep | awk '{print $2}'`
    if [ ! -z "$ifepid" ]
    then
        kill -9 $ifepid
    fi
    
    logdpid=`ps -ef | grep "$logdss" | grep -v grep | awk '{print $2}'`
    if [ ! -z "$logdpid" ]
    then
        kill -9 $logdpid
    fi
}
checksingle ()
{
    if [ -s "stack.pid" ]
    then
	oldpid=`cat stack.pid`
	chk=`ps -f -p $oldpid | grep "$scriptname" | awk '{print $2}'`
	if [ -n "$chk" ]
	then
	    echo "watch script already running"
	    exit 13
	else
	    echo $$ > stack.pid
	fi
    else
	echo $$ > stack.pid	
    fi
}
tryrestart ()
{
    if [ -s "stack.pid" ]
    then
	oldpid=`cat stack.pid`
	chk=`ps -f -p $oldpid | grep "$scriptname" | awk '{print $2}'`
	if [ -n "$chk" ]
	then
            kill -USR1 ${chk}
	    exit 13
	fi
    fi
}
repair ()
{
        bepid=`ps -ef | grep "$bess" | grep -v grep | awk '{print $2}'`
        logdpid=`ps -ef | grep "$logdss" | grep -v grep | awk '{print $2}'`
        ifepid=`ps -ef | grep "$ifess" | grep -v grep | awk '{print $2}'`
    
	
        if [ -z "$ifepid" ]
    	    then
		stopall
		trystart
	elif [ -z "$bepid" ]
	    then
		stopall
		trystart
        elif [ -z "$logdpid" ]
    	    then
		eval $logd
		logdp=`ps -ef | grep "$logdss" | grep -v grep | awk '{print $2}'`
		echo "logd:$logdp"
		cd $curwd
        fi
}
trystart ()
{
    while true
    do
	startall
        stk=$?
	if [ $stk -eq 0 ]
        then
    	    break
	else
	    echo "stack starting failed, sleep and try later"
	    sleep 60
	fi
    done
}
case "$1" in
'start')

#    echo $be
#    echo $logd
#    echo $ife
    checksingle
    stopall
    trystart
    trap "dmb='KILLED'" CHLD
    trap "status='RESTART'" USR1
    status="START"
    while true;
    do
        wait
#	echo "after wait status=$status"
	if [ $status = "START" ]
	    then
		repair
	elif [ $status = "RESTART" ]
	    then
		stopall
		trystart
		status="START"
	fi 
    done    
    ;;
'restart')
    tryrestart
    ;;
*)
    echo "Usage: $0 { start | restart}"
    ;;
esac
		
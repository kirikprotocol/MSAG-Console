#!/bin/bash
#$Id$
#
#test SAA recording at different cases
# of flooder-smsc-msag-t

base=/Users/opa/distr/msag2/services/msag

checkResults(){
  #sleep 10 #wait for sync() saa logs
  ~/build/msag2/bin/scag/tests/dumpSAAdata --CSV $base/storage/statistics/saa/*
  echo
  [ "_$showLogs" == "_YES" ] && { 
    echo "============================["
    cat $base/logs/msag.log
    echo "============================]"
  }
}

isMyMsagRunning(){
  ps -f | grep -v grep | egrep  ' bin/msag$| bin/msag | bin/msag2std | bin/msag2opts' > /dev/null
}

prepare(){
  ulimit -n 8000
  msagCmd=$*
  isMyMsagRunning  && killall msag msag2std msag2opts
  echo -n "kill old msags: "
  while  isMyMsagRunning ; do
    echo -n "."
    sleep 1
  done
  echo
  echo -n "done: "
  ps -f|grep -v grep|grep msag
}

checkMsag(){
  ps -p $mpid >/dev/null
}

startMsag(){
  #run prepare() at script start
  rm $base/storage/statistics/saa/*
  rm $base/logs/msag.log
  pushd $base >/dev/null
    $msagCmd &
    mpid=$!
  popd >/dev/null
  sleep 5
  ps -p $mpid > /dev/null || {
    echo msag not started!!!!!
    exit 1
  }
}

stopMsag(){
  kill $mpid
  for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do
    checkMsag || { echo; return; }
    echo -n .
    sleep 1
  done
  echo " Msag not stopped!! Aborting!!";
  exit 333
}

startT(){
  ./t t_point -a 07 -p 2  -h localhost:23830 -c $@ >t.out &
  tpid=$!
  sleep 1
}

stopT(){
  [ -n "$tpid" ] && ps -p $tpid >/dev/null || return
  kill $tpid
  unset tpid
  sleep 1
}

runTest(){
  local delay=$1
  shift
  startMsag
  eval $*
  sleep $delay
  stopT
  stopMsag
  checkResults
}

doFlood(){
  ./flooder flood-$1.rc >flood.out
}

checkFlood(){
  grep -q  "Command create:.*(DELIVERY)"  $base/logs/msag.log
}

flood(){
  for i in 1 2 3; do
    doFlood $1
    sleep 2
    checkFlood && return
    echo WARN: resend flood
  done
  return 333
}

cmpTest(){
  local delay=$1
  shift
  echo
  echo
  echo ">>> test " $1
  shift
  msagCmd="bin/msag2std"
  runTest $delay "$@"
  echo ===========================================================
  msagCmd="bin/msag2new"
  runTest $delay "$@"
}

checkset (){
  cmpTest 5 "SME not connected (1028)" "flood 1"
  cmpTest 5 "No Route (1025)" "flood 08-1"
  cmpTest 5 "OK transfer" "startT;flood 1"
  cmpTest 5 "temp error" "startT -e 100; flood 1"
  cmpTest 5 "perm error" "startT -r 100; flood 1"
  cmpTest 130 "no response" "startT -n 100; flood 1; echo 'wait 110 sec ...'"
  cmpTest 150 "very delayed response" "startT -l 130000; flood 1; echo 'wait 120 sec ...'"

  #test error in rules and long calls
}

showLogs=NO
[ "_$1" ==  "_-v" ] && {  shift; showLogs=YES; }
prepare
checkset


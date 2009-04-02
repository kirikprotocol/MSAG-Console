pids=`ps aux|grep tomcat|grep java|awk '{print $2}'`
WC=`echo $pids | wc -w`
[ $WC -gt 0 ] && echo "$WC processes thread dump requested" && kill -3 $pids
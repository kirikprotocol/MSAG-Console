if [ -e "tomcat.pid" ] ; then
  echo "Tomcat already started"
  exit 1
fi
export LC_ALL=@LC_ALL@
export LANG=@LC_ALL@
export LANGUAGE=@LC_ALL@
export NLS_LANG=@NSL_LANG@
nohup tomcat/tomcat_runner.sh run tomcat.pid &
echo $! > tomcat.pid

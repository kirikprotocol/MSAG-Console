if [ -e "daemon.pid" ] ; then
  echo "Daemon already started"
  exit 1
fi
export LC_ALL=@LC_ALL@
export LANG=@LC_ALL@
export LANGUAGE=@LC_ALL@
export NLS_LANG=@NLS_LANG@
cd @daemon.dir@
nohup bin/daemon.sh daemon.pid &
echo $! > daemon.pid

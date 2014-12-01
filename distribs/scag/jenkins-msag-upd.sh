rm -rf $WORKSPACE/.distr || true
mkdir $WORKSPACE/.distr || true

echo "25.08.2014 https://bugzilla.aurorisoft.com/show_bug.cgi?id=12782" > $WORKSPACE/.distr/readme.txt

mkdir $WORKSPACE/.distr/lib || true
mkdir $WORKSPACE/.distr/services || true
mkdir $WORKSPACE/.distr/services/msag || true
mkdir $WORKSPACE/.distr/services/msag/bin || true

cp $WORKSPACE/.build/msag/bin/liblogger.so $WORKSPACE/.distr/lib
cp $WORKSPACE/.build/msag/bin/scag/msag $WORKSPACE/.distr/services/msag/bin
if [ "-$XERCES_HOME" != "-" ] ; then
cp $XERCES_HOME/lib/libxerces-c-3.1.so $WORKSPACE/.distr/lib
fi

rm -rf $WORKSPACE/.distr || true
mkdir $WORKSPACE/.distr || true

echo "27.05.2015 Fixed: smpp dump in log files (bug#16277)" > $WORKSPACE/.distr/readme.txt

mkdir $WORKSPACE/.distr/lib || true
mkdir $WORKSPACE/.distr/services || true
mkdir $WORKSPACE/.distr/services/msag || true
mkdir $WORKSPACE/.distr/services/msag/bin || true

cp $WORKSPACE/.build/msag/bin/liblogger.so $WORKSPACE/.distr/lib
cp $WORKSPACE/.build/msag/bin/scag/msag $WORKSPACE/.distr/services/msag/bin
if [ "-$XERCES_HOME" != "-" ] ; then
cp $XERCES_HOME/lib/libxerces-c-3.1.so $WORKSPACE/.distr/lib
fi

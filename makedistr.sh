mkdir $HOME/distr
DISTR_DIR=$HOME/distr
echo Creating directory structure
mkdir $DISTR_DIR/bin
mkdir $DISTR_DIR/conf
mkdir $DISTR_DIR/lib
mkdir $DISTR_DIR/logs
mkdir $DISTR_DIR/services
mkdir $DISTR_DIR/temp
mkdir $DISTR_DIR/webapps
mkdir $DISTR_DIR/work
mkdir $DISTR_DIR/services/SMSC
mkdir $DISTR_DIR/services/SMSC/bin
ln -s ../../conf $DISTR_DIR/services/SMSC/conf
ln -s ../../lib $DISTR_DIR/services/SMSC/lib
ln -s ../../logs $DISTR_DIR/services/SMSC/logs
mkdir $DISTR_DIR/webapps/smsc
mkdir $DISTR_DIR/webapps/smsc/perfmon
mkdir $DISTR_DIR/webapps/smsc/WEB-INF
mkdir $DISTR_DIR/webapps/smsc/WEB-INF/lib
mkdir $DISTR_DIR/webapps/smsc/WEB-INF/classes
mkdir $DISTR_DIR/webapps/smsc/WEB-INF/classes/dtds
echo Copying dtd files
cp $SMSC_SRCDIR/../config/*.dtd $DISTR_DIR/conf
cp $SMSC_SRCDIR/../config/*.dtd $DISTR_DIR/webapps/smsc/WEB-INF/classes/dtds
cp $SMSC_SRCDIR/../config/resourcemanager/*.dtd $DISTR_DIR/conf
cp $SMSC_SRCDIR/../config/resourcemanager/*.dtd $DISTR_DIR/webapps/smsc/WEB-INF/classes/dtds
echo Copying libraries
cp $SMSC_BUILDDIR/bin/liblogger.so $DISTR_DIR/lib
cp $SMSC_BUILDDIR/bin/db/exceptions/libdb_exc.so $DISTR_DIR/lib
cp $SMSC_BUILDDIR/bin/db/oci/libdb_oci.so $DISTR_DIR/lib
echo Copying binaries
cp $SMSC_BUILDDIR/bin/admin/daemon/smsc_ssdaemon $DISTR_DIR/bin
cp $SMSC_BUILDDIR/bin/system/smsc $DISTR_DIR/services/SMSC/bin
echo Copying web administration files
ORGDIR=`pwd`
cd $DISTR_DIR
tar -xf $SMSC_SRCDIR/../config/tomcat/tomcat.tar
cd $ORGDIR
cp $SMSC_SRCDIR/webapp/smsc/WEB-INF/lib/*.jar $DISTR_DIR/webapps/smsc/WEB-INF/lib
cp $SMSC_BUILDDIR/jars/smsc.jar $DISTR_DIR/webapps/smsc/WEB-INF/lib
cp $SMSC_BUILDDIR/jars/smsc_tomcat_common.jar $DISTR_DIR/tomcat/common/lib
cp $SMSC_BUILDDIR/jars/smsc_tomcat_server.jar $DISTR_DIR/tomcat/server/lib
cp $SMSC_BUILDDIR/jars/perfmon.jar  $DISTR_DIR/webapps/smsc/perfmon
echo Copying web admin resources
cd $SMSC_SRCDIR/webapp
find . -type f -name "*.jsp" | cpio -pud $DISTR_DIR/webapps
find . -type f -name "*.js" | cpio -pud $DISTR_DIR/webapps
find . -type f -name "*.css" | cpio -pud $DISTR_DIR/webapps
find . -type f -name "*.gif" | cpio -pud $DISTR_DIR/webapps
find . -type f -name "*.jpg" | cpio -pud $DISTR_DIR/webapps
find . -type f -name "*.htc" | cpio -pud $DISTR_DIR/webapps

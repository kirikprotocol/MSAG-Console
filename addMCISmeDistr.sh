DISTR_DIR=$HOME/distr
ESME_NAME=MCISme
ESME_JAR_NAME=mcisme.jar
ESME_BIN_NAME=$SMSC_BUILDDIR/bin/mcisme/MCISme

mkdir $DISTR_DIR/services/${ESME_NAME}
mkdir $DISTR_DIR/services/${ESME_NAME}/bin
cp $ESME_BIN_NAME $DISTR_DIR/services/${ESME_NAME}/bin

SME_SRCDIR=$HOME/smsc/ext_smes/${ESME_NAME}
SME_DISTRJSPDIR=$DISTR_DIR/webapps/smsc/esme_${ESME_NAME}
mkdir $SME_DISTRJSPDIR

cp $SMSC_BUILDDIR/jars/${ESME_JAR_NAME} $DISTR_DIR/webapps/smsc/WEB-INF/lib
cd $SME_SRCDIR/jsp
find . -type f -name "*.jsp" | cpio -pud $SME_DISTRJSPDIR

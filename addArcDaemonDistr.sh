DISTR_DIR=$HOME/distr
ESME_NAME=ArchiveDaemon
ESME_BIN_NAME=$SMSC_BUILDDIR/bin/store/ArchiveDaemon

mkdir $DISTR_DIR/services/${ESME_NAME}
mkdir $DISTR_DIR/services/${ESME_NAME}/bin
cp $ESME_BIN_NAME $DISTR_DIR/services/${ESME_NAME}/bin

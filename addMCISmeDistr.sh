DISTR_DIR=$HOME/distr
ESME_NAME=MCISme
ESME_BIN_NAME=$SMSC_BUILDDIR/bin/mcisme/MCISme

mkdir $DISTR_DIR/services/${ESME_NAME}
mkdir $DISTR_DIR/services/${ESME_NAME}/bin
cp $ESME_BIN_NAME $DISTR_DIR/services/${ESME_NAME}/bin

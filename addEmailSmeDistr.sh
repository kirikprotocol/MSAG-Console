DISTR_DIR=$HOME/distr
ESME_NAME=emailsme
ESME_BIN_NAME=$SMSC_BUILDDIR/bin/emailsme/emailsme
AGENT_BIN_NAME=$SMSC_BUILDDIR/bin/emailsme/mdAgent

mkdir $DISTR_DIR/services/${ESME_NAME}
mkdir $DISTR_DIR/services/${ESME_NAME}/bin
cp $ESME_BIN_NAME $DISTR_DIR/services/${ESME_NAME}/bin
cp $AGENT_BIN_NAME $DISTR_DIR/services/${ESME_NAME}/bin
cp $SMSC_SRCDIR/emailsme/mailreaper.pl $DISTR_DIR/services/${ESME_NAME}/bin
chmod +x $DISTR_DIR/services/${ESME_NAME}/bin/mailreaper.pl
 
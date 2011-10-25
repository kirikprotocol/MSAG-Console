import mobi.eyeline.smpp.api.PDUListener;
import mobi.eyeline.smpp.api.SmppClient;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.pdu.*;
import mobi.eyeline.smpp.api.pdu.data.InvalidAddressFormatException;
import mobi.eyeline.smpp.api.types.RegDeliveryReceipt;
import mobi.eyeline.smpp.api.types.Status;
import org.apache.log4j.Logger;

import java.io.IOException;
import java.io.InputStream;
import java.text.DateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Properties;

/**
 * Send SubmitSM and don't answer DeliverSMResp three time with temporal error status, then receive fourth DeliverSM
 * answer DeliverSMResp with status OK.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 28.02.11
 * Time: 10:30
 */
public class TempErrorTest extends Thread implements PDUListener {

    static Logger logger = Logger.getLogger(Client.class);
    Properties config;
    SmppClient smppClient;

    private static String dest_address, source_address;
    private static long validity_period;

    private int counter = 0;

    private static String con;

    public TempErrorTest() throws SmppException {
        this.config = new Properties();
        try {
          InputStream is = this.getClass().getResourceAsStream("/config.properties");
          config.load(is);
          logger.debug("Successfully load config properties.");
        } catch (IOException e) {
          throw new SmppException("Could not initialize", e);
        }

        logger.debug("Try to add shutdown hook ...");
        Runtime.getRuntime().addShutdownHook(this);

        logger.debug("Try to initialize smpp client ...");
        smppClient = new SmppClient(config, this);
        logger.debug("Try to start smpp client ...");
        smppClient.start();

        source_address = config.getProperty("source.address");
        dest_address = config.getProperty("dest.address");
        validity_period = Long.parseLong(config.getProperty("validity.period"));

        con = config.getProperty("con");

        logger.debug("Start smpp client.");
    }

    public boolean handlePDU(PDU pdu) {
        logger.debug("Handle pdu with '"+pdu.getType()+"' type.");
        switch (pdu.getType()) {
            case SubmitSM:
                try {
                    Message request = (Message)pdu;
                    smppClient.send(request);
                    logger.debug("Send SubmitSM request.");
                    return true;
                } catch (SmppException e) {
                    logger.error("", e);
                    return false;
                }


            case DeliverSM:

                counter++;
                if (counter < 3){
                    DeliverSM deliverSM = (DeliverSM) pdu;
                    int sequence_number = deliverSM.getSequenceNumber();
                    logger.debug("Handle DeliverSM pdu with sequence number "+sequence_number);

                    DeliverSMResp resp = new DeliverSMResp();
                    resp.setConnectionName(deliverSM.getConnectionName());
                    resp.setSequenceNumber(deliverSM.getSequenceNumber());
                    resp.setStatus(Status.RX_T_APPN);

                    try{
                        smppClient.send(resp);
                        return true;
                    } catch (SmppException e){
                        logger.error("Couldn't send DeliverSMResp.", e);
                        return false;
                    }
                } else {
                    DeliverSM deliverSM = (DeliverSM) pdu;
                    int sequence_number = deliverSM.getSequenceNumber();
                    logger.debug("Handle DeliverSM pdu with sequence number "+sequence_number);

                    DeliverSMResp resp = new DeliverSMResp();
                    resp.setConnectionName(deliverSM.getConnectionName());
                    resp.setSequenceNumber(deliverSM.getSequenceNumber());
                    resp.setStatus(Status.OK);

                    try{
                        smppClient.send(resp);
                        logger.debug("Temporal error test - OK");
                        System.exit(0);
                        return true;
                    } catch (SmppException e){
                        logger.error("Couldn't send DeliverSMResp.", e);
                        return false;
                    }
                }

            case SubmitSMResp:

                SubmitSMResp submitSMResp = (SubmitSMResp) pdu;
                String message_id_str = submitSMResp.getMessageId();
                if (submitSMResp.getStatus() == Status.OK){
                    logger.debug("SubmitSMResp messageId="+message_id_str);
                } else {
                    logger.debug("SubmitSMResp status="+submitSMResp.getStatus());
                }

        }
        return false;
    }

    public static void main(String args[]){
        logger.debug("Start smpp client.");
        try{
            final TempErrorTest client = new TempErrorTest();

            SubmitSM submitSM = new SubmitSM();
            submitSM.setRegDeliveryReceipt(RegDeliveryReceipt.SuccessOrFailure);

            submitSM.setConnectionName(con);

            DateFormat df = DateFormat.getDateTimeInstance();
            Calendar cal = Calendar.getInstance();
            Date date = cal.getTime();

            submitSM.setMessage("message from "+submitSM.getConnectionName()+", "+df.format(date));

            try{
                submitSM.setSourceAddress(source_address);
                submitSM.setDestinationAddress(dest_address);
            } catch (InvalidAddressFormatException e){
                logger.error(e);
            }
            submitSM.setValidityPeriod(1000 * validity_period);
            client.handlePDU(submitSM);

        } catch (SmppException e) {
            logger.error("", e);
        }
    }

    @Override
    public void run() {
      if( smppClient != null ) smppClient.shutdown();
    }
}


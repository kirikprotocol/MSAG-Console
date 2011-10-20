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

public class CancelTest extends Thread implements PDUListener {

    static Logger logger = Logger.getLogger(Client.class);
    Properties config;
    SmppClient smppClient;

    private static String dest_address, source_address;
    private static long validity_period;

    private static String con;

    private String message;

    public CancelTest() throws SmppException {
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

        message = config.getProperty("message");
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

                //counter++;
                //if (counter > 1){

                    DeliverSM deliverSM = (DeliverSM) pdu;
                    int sequence_number = deliverSM.getSequenceNumber();
                    logger.debug("Handle DeliverSM pdu with sequence number "+sequence_number);

                    try{
                        logger.debug("123: "+deliverSM.getResponse().getSequenceNumber());
                        smppClient.send(deliverSM.getResponse());
                        logger.debug("Send DeliverSMResp pdu.");
                        return true;
                    } catch (SmppException e){
                        logger.error("Couldn't send DeliverSMResp.", e);
                        return false;
                    }

                //} else {
                //    return true;
                //}

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
            final CancelTest client = new CancelTest();

            int count = Integer.parseInt(args[0]);

            for(int i=0; i < count; i++){
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
            }

            System.exit(0);

        } catch (SmppException e) {
            logger.error("", e);
        }
    }

    @Override
    public void run() {
      if( smppClient != null ) smppClient.shutdown();
    }
}

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
import java.util.*;

/**
 *
 * User: Stepanov Dmitry Nikolaevich
 * Date: 28.02.11
 * Time: 10:30
 */
public class ResendTest2 extends Thread implements PDUListener {

    static Logger logger = Logger.getLogger(Client.class);
    Properties config;
    SmppClient smppClient;

    private static String dest_address, source_address;
    private static long validity_period;

    private static String con;

    public ResendTest2() throws SmppException {
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
            case SubmitSM: {
                try {

                    Message request = (Message)pdu;
                    smppClient.send(request);
                    logger.debug("Send SubmitSM request.");
                    return true;
                } catch (SmppException e) {
                    logger.error("", e);
                    return false;
                }
            }

            case DeliverSM: {

                DeliverSM deliverSM = (DeliverSM) pdu;
                String message = deliverSM.getMessage();
                String[] ar = message.split(",");
                String s = ar[0];
                long message_id = Long.parseLong(s.substring(s.indexOf(":")+1,s.length()));
                int v = ids_set.get(message_id);

                if (v == 0) {
                    ids_set.put(message_id, 1);
                    logger.debug("receive DeliverSM with message_id "+message_id+ " first time.");
                    return true;
                } else if ( v == 1 ) {
                    ids_set.put(message_id, 2);
                    logger.debug("receive DeliverSM with message_id "+message_id+ " second time.");

                    int sequence_number = deliverSM.getSequenceNumber();
                    logger.debug("Handle DeliverSM pdu with sequence number "+sequence_number);

                    DeliverSMResp resp = new DeliverSMResp();
                    resp.setConnectionName(deliverSM.getConnectionName());
                    resp.setSequenceNumber(deliverSM.getSequenceNumber());
                    resp.setStatus(Status.OK);

                    try{
                        smppClient.send(resp);

                        boolean all_received = true;
                        for(Long id : ids_set.keySet()){
                            if (ids_set.get(id) != 2) all_received = false;
                        }

                        if (all_received){
                            logger.debug("Resent test 2 - OK");
                            System.exit(0);
                        }
                        return true;
                    } catch (SmppException e){
                        logger.error("Couldn't send DeliverSMResp.", e);
                        return false;
                    }

                } else {
                    logger.error("error");
                    System.exit(2);
                }
            }

            case SubmitSMResp: {

                SubmitSMResp submitSMResp = (SubmitSMResp) pdu;
                long message_id;
                if (submitSMResp.getMessageId() != null){
                    message_id = Long.parseLong(submitSMResp.getMessageId());
                    ids_set.put(message_id, 0);
                    logger.debug("receive SubmitSMResp with message_id "+message_id);
                }
            }

        }
        return false;
    }

    static int sended;
    private static Hashtable<Long, Integer> ids_set = new Hashtable<Long, Integer>();

    public static void main(String args[]) throws InterruptedException {
        logger.debug("Start smpp client.");
        try{
            final ResendTest2 client = new ResendTest2();

            sended = Integer.parseInt(args[0]);
            for(int i=0; i < sended; i++){
                SubmitSM submitSM = new SubmitSM();
                submitSM.setRegDeliveryReceipt(RegDeliveryReceipt.SuccessOrFailure);

                submitSM.setConnectionName(con);

                submitSM.setMessage("message");

                try{
                    submitSM.setSourceAddress(source_address);
                    submitSM.setDestinationAddress(dest_address);
                } catch (InvalidAddressFormatException e){
                    logger.error(e);
                }
                submitSM.setValidityPeriod(1000 * validity_period);
                client.handlePDU(submitSM);
            }
        } catch (SmppException e) {
            logger.error("", e);
        }

        Thread.sleep(600000);
        boolean all_received = true;
        for(Long id : ids_set.keySet()){
            if (ids_set.get(id) == 0) all_received = false;
        }

        if (all_received){
            logger.debug("Resent test 2 - OK");
            System.exit(0);
        } else {
            logger.debug("Test failed.");
            System.exit(1);
        }
    }

    @Override
    public void run() {
      if( smppClient != null ) smppClient.shutdown();
    }
}


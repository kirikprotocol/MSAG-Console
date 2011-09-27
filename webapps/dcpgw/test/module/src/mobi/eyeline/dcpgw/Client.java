package mobi.eyeline.dcpgw;

import mobi.eyeline.smpp.api.PDUListener;
import mobi.eyeline.smpp.api.SmppClient;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.pdu.*;
import org.apache.log4j.Logger;
import java.util.Hashtable;
import java.util.Properties;


/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 28.02.11
 * Time: 10:30
 */
public class Client extends Thread implements PDUListener {

    static Logger logger = Logger.getLogger(Client.class);
    Properties config;
    SmppClient smppClient;

    Hashtable<Integer, PDU> sequence_number_submit_sm_resp_table;
    Hashtable<Integer, PDU> sequence_number_deliver_sm_table;

    public Client(Properties config) throws SmppException {
        this.config = config;
        Runtime.getRuntime().addShutdownHook(this);
        smppClient = new SmppClient(config, this);
        logger.debug("Try to start smpp client ...");
        smppClient.start();

        sequence_number_submit_sm_resp_table = new Hashtable<Integer, PDU>();
        sequence_number_deliver_sm_table = new Hashtable<Integer, PDU>();
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


                    DeliverSM deliverSM = (DeliverSM) pdu;
                    int sequence_number = deliverSM.getSequenceNumber();
                    logger.debug("Handle DeliverSM pdu with sequence number "+sequence_number);

                    try{
                        logger.debug("123: "+deliverSM.getResponse().getSequenceNumber());
                        smppClient.send(deliverSM.getResponse());
                        //sequence_number_deliver_sm_table.put(deliverSM.ge);
                        logger.debug("Send DeliverSMResp pdu.");
                        return true;
                    } catch (SmppException e){
                        logger.error("Couldn't send DeliverSMResp.", e);
                        return false;
                    }

            case SubmitSMResp:

                SubmitSMResp submitSMResp = (SubmitSMResp) pdu;
                String message_id_str = submitSMResp.getMessageId();
                logger.debug("SubmitSMResp messageId="+message_id_str);
                long message_id = Long.parseLong(message_id_str);
                logger.debug("message_id="+message_id);
                sequence_number_submit_sm_resp_table.put(submitSMResp.getSequenceNum(), submitSMResp);

        }
        return false;
    }

    @Override
    public void run() {
      if( smppClient != null ) smppClient.shutdown();
    }
}

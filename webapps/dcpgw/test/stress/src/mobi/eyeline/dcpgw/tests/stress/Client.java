package mobi.eyeline.dcpgw.tests.stress;

import mobi.eyeline.smpp.api.PDUListener;
import mobi.eyeline.smpp.api.SmppClient;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.pdu.*;
import mobi.eyeline.smpp.api.pdu.data.InvalidAddressFormatException;
import mobi.eyeline.smpp.api.types.RegDeliveryReceipt;
import mobi.eyeline.smpp.api.types.Status;
import org.apache.log4j.Logger;
import org.junit.Assert;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.text.DateFormat;
import java.util.*;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 28.02.11
 * Time: 10:30
 */
public class Client extends Thread implements PDUListener {

    static Logger log = Logger.getLogger(Client.class);
    Properties config;
    SmppClient smppClient;

    HashSet<Integer> submit_sm_sequence_number_set;

    HashSet<Long> message_ids;

    int cs1, cs2, cd1;

    public Client(Properties config) throws SmppException {
        this.config = config;

        log.debug("Try to add shutdown hook ...");
        Runtime.getRuntime().addShutdownHook(this);

        log.debug("Try to initialize smpp client ...");
        smppClient = new SmppClient(config, this);
        log.debug("Try to start smpp client ...");
        smppClient.start();

        submit_sm_sequence_number_set = new HashSet<Integer>();
        message_ids = new HashSet<Long>();

        log.debug("Start smpp client.");
    }

    public boolean handlePDU(PDU pdu) {
        log.debug("Handle pdu with '"+pdu.getType()+"' type.");
        switch (pdu.getType()) {
            case SubmitSM: {
                try {
                    Message request = (Message)pdu;
                    smppClient.send(request);
                    cs1++;
                    int sn = pdu.getSequenceNumber();
                    submit_sm_sequence_number_set.add(sn);
                } catch (SmppException e) {
                    log.error(e);
                    return false;
                }
                break;
            }
            case DeliverSM: {

                    cd1++;
                    DeliverSM deliverSM = (DeliverSM) pdu;
                    String message = deliverSM.getMessage();
                    String[] ar = message.split("\\s");
                    String s = ar[0];
                    int sn = deliverSM.getSequenceNumber();


                    long message_id = Long.parseLong(s.substring(s.indexOf(":")+1,s.length()));

                    boolean contain = message_ids.remove(message_id);

                    DeliverSMResp deliverSMResp = new DeliverSMResp();
                    deliverSMResp.setStatus(Status.OK);
                    deliverSMResp.setSequenceNumber(sn);
                    deliverSMResp.setConnectionName(deliverSM.getConnectionName());

                    //Assert.assertTrue("Unknown message id.", contain);

                    try{
                        smppClient.send(deliverSMResp);
                    } catch (SmppException e){
                        log.error("Couldn't send DeliverSMResp.", e);
                        return false;
                    }
                break;
            }

            case SubmitSMResp: {
                cs2++;
                SubmitSMResp submitSMResp = (SubmitSMResp) pdu;
                String message_id_str = submitSMResp.getMessageId();
                int sn = submitSMResp.getSequenceNumber();
                log.debug("receive SubmitSMResp: id="+message_id_str+", sn:"+sn);

                Assert.assertNotNull("Message id null", message_id_str);

                long message_id = Long.parseLong(submitSMResp.getMessageId());
                message_ids.add(message_id);

                boolean contain = submit_sm_sequence_number_set.remove(sn);

                Assert.assertTrue("Unknown SubmitSMResp sequence number.", contain);

                break;
            }
        }
        log.debug("SubmitSM: "+cs1+", SubmitSMResp: "+cs2+", DeliverSM: "+cd1);
        return false;
    }

    @Override
    public void run() {
        if( smppClient != null ) smppClient.shutdown();
    }

    public void shutdown(){
        if( smppClient != null ) smppClient.shutdown();
    }

    public HashSet<Integer> getSubmitSMSequenceNumberSet(){
        return submit_sm_sequence_number_set;
    }

    public HashSet<Long> getMessageIdSet(){
        return message_ids;
    }
}

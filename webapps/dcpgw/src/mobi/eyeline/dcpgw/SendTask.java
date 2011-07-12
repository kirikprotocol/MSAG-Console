package mobi.eyeline.dcpgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DcpConnection;
import mobi.eyeline.informer.admin.delivery.Message;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.SmppServer;
import mobi.eyeline.smpp.api.pdu.SubmitSMResp;
import mobi.eyeline.smpp.api.types.Status;
import org.apache.log4j.Logger;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.List;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * User: Stepanov Dmitry Nikolaevich
 * Date: 28.06.11
 * Time: 12:10
 */
public class SendTask implements Runnable{

    private static Logger log = Logger.getLogger(SendTask.class);

    private final int delivery_id;
    private Sender sender;


    public SendTask(Sender sender, int delivery_id){
        this.sender = sender;
        this.delivery_id = delivery_id;
    }

    public void run() {
        log.debug("Start send task for "+delivery_id+"_queue.");
        sender.sendMessages(delivery_id);
        log.debug("Send task for "+delivery_id+"_queue has finished.");
    }

}

package mobi.eyeline.dcpgw;

import org.apache.log4j.Logger;

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

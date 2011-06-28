package mobi.eyeline.dcpgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DcpConnection;
import mobi.eyeline.informer.admin.delivery.Message;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.SmppServer;
import mobi.eyeline.smpp.api.types.Status;
import org.apache.log4j.Logger;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * User: Stepanov Dmitry Nikolaevich
 * Date: 28.06.11
 * Time: 12:10
 */
public class SendTask implements Runnable{

    private static Logger log = Logger.getLogger(SendTask.class);

    private final LinkedBlockingQueue<Message> queue;
    private final int delivery_id;
    private DcpConnection connection;
    private SmppServer smppServer;
    private HashMap<Message, mobi.eyeline.smpp.api.pdu.Message> messages_requests_map;

    public SendTask(LinkedBlockingQueue<Message> queue, int delivery_id, DcpConnection connection,
                    SmppServer smppServer, HashMap<Message, mobi.eyeline.smpp.api.pdu.Message> messages_requests_map){
        this.queue = queue;
        this.delivery_id = delivery_id;
        this.connection = connection;
        this.smppServer = smppServer;
        this.messages_requests_map = messages_requests_map;
    }

    public void run() {
        log.debug("Start send task for "+delivery_id+"_queue.");

        if (!queue.isEmpty()){
            log.debug(delivery_id+"_queue is not empty and has size "+queue.size());

            //log.debug(delivery_id+"_queue size before conversion to array is "+queue.size());
            Message[] ar = new Message[queue.size()];
            List<Message> list = Arrays.asList(queue.toArray(ar));
            //log.debug(delivery_id+"_queue size after conversion to array is " + queue.size());

            try{
                log.debug("Try to add list with messages to delivery with id '"+delivery_id+"' ...");
                connection.addDeliveryMessages(delivery_id, list);
                log.debug("Successfully add list with messages to delivery with id '"+delivery_id+"'.");

                for(Message m: list){

                    mobi.eyeline.smpp.api.pdu.Message request = messages_requests_map.get(m);
                    log.debug("map size = "+messages_requests_map.size());
                    try{
                        smppServer.send(request.getResponse(Status.OK));
                        messages_requests_map.remove(m);

                        log.debug("Send smpp response with status 'OK' for "+m.getProperties().getProperty("gId")+" message from "+delivery_id+" delivery." );
                    } catch (SmppException e) {
                        log.error("Could not send response to client", e);
                    }

                }

                queue.clear();
                log.debug("Clean queue for delivery with '"+delivery_id+"'.");

            } catch (AdminException e) {
                log.error("Couldn't add list with messages to delivery with id '"+delivery_id+"'.",e);

                for(Message m: list){
                    mobi.eyeline.smpp.api.pdu.Message request = messages_requests_map.get(m);

                    try{
                        smppServer.send(request.getResponse(Status.SYSERR));
                        messages_requests_map.remove(m);
                    } catch (SmppException e2) {
                        log.error("Could not send response to client", e);
                    }
                }

                queue.clear();
                log.debug("Clean queue for delivery with '"+delivery_id+"'.");
            }

        } else {
            log.debug(delivery_id+"_queue is empty.");
        }

        log.debug("Send task for "+delivery_id+"_queue has finished.");
    }

}

package mobi.eyeline.dcpgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DcpConnection;
import mobi.eyeline.informer.admin.delivery.Message;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.SmppServer;
import mobi.eyeline.smpp.api.pdu.Request;
import mobi.eyeline.smpp.api.pdu.data.Address;
import mobi.eyeline.smpp.api.types.Status;
import org.apache.log4j.Logger;

import java.util.*;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 14.06.11
 * Time: 18:02
 */
public class Sender extends Thread{

    private static Logger log = Logger.getLogger(Sender.class);

    private DcpConnection connection;
    private String host;
    private int port;
    private String login;
    private String password;

    private int capacity;

    private long timeout;

    private final Map<Integer, LinkedBlockingQueue<Message>> delivery_id_queue_map;

    private boolean interrupt = false;

    private HashMap<Message, mobi.eyeline.smpp.api.pdu.Message> messages_requests_map;

    private SmppServer smppServer;

    public Sender(String host, int port, final String login, String password, int capacity, long timeout, SmppServer smppServer){

        this.host = host;
        this.port = port;
        this.login = login;
        this.password = password;

        this.capacity = capacity;

        this.timeout = timeout;
        this.smppServer = smppServer;

        delivery_id_queue_map = Collections.synchronizedMap(new HashMap<Integer, LinkedBlockingQueue<Message>>());

        messages_requests_map = new HashMap<Message, mobi.eyeline.smpp.api.pdu.Message>();
    }

    public void addMessage(int delivery_id, long gId, mobi.eyeline.smpp.api.pdu.Message request){
        log.debug("Try to put message with gateway identifier '"+gId+"' to the queue for delivery with identifier "+delivery_id+" ...");

        LinkedBlockingQueue<Message> queue = delivery_id_queue_map.get(delivery_id);
        if (queue == null){
            log.debug("Appropriate queue not found.");
            queue = new LinkedBlockingQueue<Message>(capacity);
            delivery_id_queue_map.put(delivery_id, queue);
            log.debug("Initialize new queue for delivery with identifier "+delivery_id+".");
        }

        Address smpp_destination_address = request.getDestinationAddress();
        String destination_address_str = smpp_destination_address.getAddress();
        log.debug("destination address: "+destination_address_str);

        String text = request.getMessage();
        log.debug("text: "+text);

        mobi.eyeline.informer.util.Address informer_destination_address = new mobi.eyeline.informer.util.Address(destination_address_str);

        Message informer_message = Message.newMessage(informer_destination_address, text);

        informer_message.setProperty("gId", Long.toString(gId));

        try {
            queue.put(informer_message);
            log.debug("Successfully put the message with gateway identifier '"+gId+"' to the queue for delivery with identifier '"+delivery_id+"'.");
        } catch (InterruptedException e) {
            log.error(e);
            // todo ?
        }

        messages_requests_map.put(informer_message, request);

        int size = queue.size();
        //log.debug("Queue size has increased to "+size+".");

        if (size == capacity) {
            notify();
            log.debug("Queue is full, try to notify the waiting thread to add messages to informer's deliveries ...");
        } else {
            log.debug("Queue size less than capacity, do nothing.");
        }
    }

    public void run() {
        log.debug("Start sender for user '"+login+"'.");
        while(!interrupt){

            log.debug("Check lists for user "+login+".");
            for (Map.Entry<Integer, LinkedBlockingQueue<Message>> entry : delivery_id_queue_map.entrySet()) {

                int delivery_id = entry.getKey();
                LinkedBlockingQueue<Message> queue = entry.getValue();

                log.debug("Detected what queue for delivery with id '"+delivery_id+"' is full.");
                if (connection == null){
                    try{
                        log.debug("Try to create new dcp connection for user '"+login+"' ...");
                        connection = new DcpConnection(host, port, login, password);
                        log.debug("Successfully create new dcp connection for user '"+login+"'.");
                    } catch (AdminException e) {
                        log.error("Couldn't create new dcp connection for user '"+login+"'.",e);
                        // todo ?
                    }
                }

                Message[] ar = new Message[queue.size()];
                List<Message> list = Arrays.asList(queue.toArray(ar));

                try {
                    log.debug("Try to add list with messages to delivery with id '"+delivery_id+"' ...");
                    connection.addDeliveryMessages(delivery_id, list);
                    log.debug("Successfully add list with messages to delivery with id '"+delivery_id+"'.");

                    for(Message m: list){
                        mobi.eyeline.smpp.api.pdu.Message request = messages_requests_map.get(m);

                        try{
                            smppServer.send(request.getResponse(Status.OK));
                            messages_requests_map.remove(m);
                            queue.clear();
                        } catch (SmppException e) {
                            log.error("Could not send response to client", e);
                        }

                    }

                } catch (AdminException e) {
                    log.error("Couldn't add list with messages to delivery with id '"+delivery_id+"'.",e);

                    for(Message m: list){
                        mobi.eyeline.smpp.api.pdu.Message request = messages_requests_map.get(m);

                        try{
                            smppServer.send(request.getResponse(Status.SYSERR));
                            messages_requests_map.remove(m);
                            queue.clear();
                        } catch (SmppException e2) {
                            log.error("Could not send response to client", e);
                        }

                    }
                }
            }

            try {
                log.debug("Sender for user '"+login+"' wait "+timeout+" mls.");
                wait(timeout);
            } catch (InterruptedException e) {
                log.error(e);
                // todo ?
            }
        }

    }

    public void interrupt(){
        interrupt = true;
    }



}

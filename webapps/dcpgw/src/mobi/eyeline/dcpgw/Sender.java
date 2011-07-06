package mobi.eyeline.dcpgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DcpConnection;
import mobi.eyeline.informer.admin.delivery.Message;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.SmppServer;
import mobi.eyeline.smpp.api.pdu.data.Address;
import mobi.eyeline.smpp.api.types.Status;
import org.apache.log4j.Logger;

import java.util.*;
import java.util.concurrent.*;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 14.06.11
 * Time: 18:02
 */
public class Sender extends Thread{

    private static Logger log = Logger.getLogger(Sender.class);

    private final ScheduledExecutorService scheduler = Executors.newSingleThreadScheduledExecutor();

    private DcpConnection connection;
    private String host;
    private int port;
    private String login;
    private String password;

    private int capacity;

    private long sending_timeout, waiting_timeout;

    private final Map<Integer, LinkedBlockingQueue<Message>> delivery_id_queue_map;

    private boolean interrupt = false;

    private HashMap<Message, mobi.eyeline.smpp.api.pdu.Message> messages_requests_map;

    private SmppServer smppServer;

    private Map<LinkedBlockingQueue<Message>, ScheduledFuture> queue_task_map;
    private LinkedBlockingQueue<SendTask> sendTaskQueue;

    public Sender(String host, int port, final String login, String password, int capacity, long sending_timeout, long waiting_timeout, SmppServer smppServer){
        this.host = host;
        this.port = port;
        this.login = login;
        this.password = password;

        this.capacity = capacity;

        this.sending_timeout = sending_timeout;
        this.waiting_timeout = waiting_timeout;

        this.smppServer = smppServer;

        delivery_id_queue_map = Collections.synchronizedMap(new HashMap<Integer, LinkedBlockingQueue<Message>>());
        queue_task_map = Collections.synchronizedMap(new HashMap<LinkedBlockingQueue<Message>, ScheduledFuture>());

        messages_requests_map = new HashMap<Message, mobi.eyeline.smpp.api.pdu.Message>();

        sendTaskQueue = new LinkedBlockingQueue<SendTask>();
    }

    public void addMessage(int delivery_id, long gId){
        log.debug("Try to put '"+gId+"' message to the "+delivery_id+"_queue ...");

        boolean message_added_to_queue = false;

        LinkedBlockingQueue<Message> queue;
        synchronized (delivery_id_queue_map){
            if (delivery_id_queue_map.containsKey(delivery_id)){
                queue = delivery_id_queue_map.get(delivery_id);
            } else {
                queue = new LinkedBlockingQueue<Message>(capacity);
                delivery_id_queue_map.put(delivery_id, queue);
                log.debug("Initialize new "+delivery_id+"_queue.");
            }
        }

        mobi.eyeline.smpp.api.pdu.Message request = Manager.getInstance().getRequest(gId);

        Address smpp_destination_address = request.getDestinationAddress();
        String destination_address_str = smpp_destination_address.getAddress();
        String text = request.getMessage();
        log.debug("gId '"+gId+"', destination address '"+destination_address_str+"', text '"+text+"'.");

        mobi.eyeline.informer.util.Address informer_destination_address = new mobi.eyeline.informer.util.Address(destination_address_str);

        Message informer_message = Message.newMessage(informer_destination_address, text);

        informer_message.setProperty("gId", Long.toString(gId));

        synchronized (queue){

            try {

                log.debug(delivery_id + "_queue size before putting new message " + queue.size() + ".");
                message_added_to_queue = queue.offer(informer_message, waiting_timeout, TimeUnit.MILLISECONDS);
                log.debug("Successfully put "+gId+" message to "+delivery_id+"_queue.");

            } catch (InterruptedException e) {
                log.error(e);

                try {
                    smppServer.send(request.getResponse(Status.SYSERR));
                } catch (SmppException e1) {
                    log.error(e1);
                    // todo ?
                }

            }

            if (message_added_to_queue){
                int size = queue.size();
                log.debug(delivery_id+"_queue size has increased to "+size+".");

                if (queue.size() == 1){

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

                    SendTask sendTask = new SendTask(queue, delivery_id, connection, smppServer, messages_requests_map);
                    ScheduledFuture scheduledFuture = scheduler.schedule(sendTask, sending_timeout, TimeUnit.MILLISECONDS);
                    queue_task_map.put(queue, scheduledFuture);
                }

                messages_requests_map.put(informer_message, request);

                if (size == capacity) {
                    ScheduledFuture scheduledFuture = queue_task_map.get(queue);
                    scheduledFuture.cancel(true);
                    queue_task_map.remove(queue);

                    SendTask sendTask = new SendTask(queue, delivery_id, connection, smppServer, messages_requests_map);
                    sendTaskQueue.add(sendTask);

                    synchronized (this){
                        log.debug("Try to notify all waited threads ...");
                        notifyAll();
                    }
                    log.debug(delivery_id+"_queue is full, try to notify the waiting thread to add messages to informer's deliveries ...");

                } else if (size < capacity) {
                    log.debug(delivery_id+"_queue size "+size+" less than capacity "+capacity+", do nothing.");
                } else if (size > capacity) {
                    log.error(delivery_id+"_queue size '"+size+"' more than capacity '"+capacity+".");
                }
            }

        }
    }

    public void run() {
        log.debug("Start sender for user '"+login+"'.");
        while(!interrupt){

            log.debug("Iterate queues for user "+login+".");

            while(!sendTaskQueue.isEmpty()){
                SendTask sendTask = sendTaskQueue.poll();
                sendTask.run();
            }

            synchronized (this){
                try{
                    log.debug("Sender for user '"+login+"' wait.");
                    wait();
                } catch (InterruptedException e) {
                    log.error(e);
                    // todo ?
                }
            }
        }
        log.debug("Sender for user '"+login+"' has finished.");
    }

    public void interrupt(){
        interrupt = true;
    }



}

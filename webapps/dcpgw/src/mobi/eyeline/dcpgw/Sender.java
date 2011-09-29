package mobi.eyeline.dcpgw;

import mobi.eyeline.dcpgw.dcp.DcpConnection;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.SmppServer;
import mobi.eyeline.smpp.api.pdu.SubmitSMResp;
import mobi.eyeline.smpp.api.types.Status;
import org.apache.log4j.Logger;

import java.text.SimpleDateFormat;
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

    private int capacity;

    private long sending_timeout;

    private final Map<Integer, LinkedBlockingQueue<Message>> delivery_id_queue_map;

    private SmppServer smppServer;

    private Map<LinkedBlockingQueue<Message>, ScheduledFuture> queue_task_map;
    private LinkedBlockingQueue<SendTask> sendTaskQueue;

    private Hashtable<Long, Integer> id_seq_num_table;
    private Hashtable<Long, String> id_conn_name_table;

    private Calendar cal = Calendar.getInstance();

    private static SimpleDateFormat sdf = new SimpleDateFormat("yyMMddHHmmssSSS");

    public Sender(DcpConnection dcpConnection, int capacity, long sending_timeout){
        log.debug("Try to initialize sender ...");
        this.connection = dcpConnection;

        this.capacity = capacity;

        this.sending_timeout = sending_timeout;

        delivery_id_queue_map = Collections.synchronizedMap(new HashMap<Integer, LinkedBlockingQueue<Message>>());
        queue_task_map = Collections.synchronizedMap(new HashMap<LinkedBlockingQueue<Message>, ScheduledFuture>());

        sendTaskQueue = new LinkedBlockingQueue<SendTask>();

        id_seq_num_table = new Hashtable<Long, Integer>();
        id_conn_name_table = new Hashtable<Long, String>();
    }

    public void setSmppServer(SmppServer smppServer){
        this.smppServer = smppServer;
    }

    public void setDcpConnection(DcpConnection connection){
        this.connection = connection;
    }

    public DcpConnection getDcpConnection(){
        return connection;
    }

    public void addMessage(long id,
                           String source_address,
                           String destination_address,
                           String text,
                           int sequence_number,
                           String connection_name,
                           int delivery_id){
        log.debug("Try to put '"+id+"' message to the "+delivery_id+"_queue ...");

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

        mobi.eyeline.informer.util.Address informer_destination_address = new mobi.eyeline.informer.util.Address(destination_address);

        Message informer_message = Message.newMessage(informer_destination_address, text);

        informer_message.setProperty("id", Long.toString(id));
        informer_message.setProperty("sa", source_address);
        informer_message.setProperty("con", connection_name);

        synchronized (queue){

            try {

                log.debug(delivery_id + "_queue size before putting new message " + queue.size() + ".");
                queue.put(informer_message);
                id_seq_num_table.put(id, sequence_number);
                id_conn_name_table.put(id, connection_name);
                log.debug("Successfully put "+id+" message to "+delivery_id+"_queue.");

            } catch (InterruptedException e) {
                log.error(e);

                try {
                    SubmitSMResp submitSMResp = new SubmitSMResp();
                    submitSMResp.setConnectionName(connection_name);
                    submitSMResp.setStatus(Status.SYSERR);
                    submitSMResp.setSequenceNumber(sequence_number);
                    smppServer.send(submitSMResp, false);
                } catch (SmppException e1) {
                    log.error(e1);
                    // todo ?
                }

            }

            int size = queue.size();
            log.debug(delivery_id+"_queue size has increased to "+size+".");

            if (queue.size() == 1){
                SendTask sendTask = new SendTask(this, delivery_id);
                ScheduledFuture scheduledFuture = scheduler.schedule(sendTask, sending_timeout, TimeUnit.MILLISECONDS);
                queue_task_map.put(queue, scheduledFuture);
            }


            if (size == capacity) {
                ScheduledFuture scheduledFuture = queue_task_map.get(queue);
                scheduledFuture.cancel(true);
                queue_task_map.remove(queue);

                SendTask sendTask = new SendTask(this, delivery_id);
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

    public void sendMessages(int delivery_id){
        LinkedBlockingQueue<Message> queue = delivery_id_queue_map.get(delivery_id);

        if (!queue.isEmpty()){
            log.debug(delivery_id+"_queue is not empty and has size "+queue.size());

            Message[] ar = new Message[queue.size()];
            List<Message> list = Arrays.asList(queue.toArray(ar));

            for(Message m: list){
                Date date = cal.getTime();
                m.setProperty("sd", sdf.format(date));
            }

            try{
                log.debug("Try to add list with messages to delivery with id '"+delivery_id+"' ...");

                long[] informer_message_ids = connection.addDeliveryMessages(delivery_id, list);
                log.debug("Successfully add list with messages to delivery with id '"+delivery_id+"'.");

                DeliveryStatistics delivery_statistics = connection.getDeliveryState(delivery_id);
                DeliveryState ds = delivery_statistics.getDeliveryState();

                if (ds.getStatus() == DeliveryStatus.Finished){
                    log.debug("Detected that delivery status is finished.");
                    DeliveryState deliveryState = new DeliveryState();
                    deliveryState.setStatus(DeliveryStatus.Planned);
                    connection.changeDeliveryState(delivery_id, deliveryState);
                    log.debug("Change delivery status on planned.");
                }

                for(int i = 0; i < list.size(); i++ ){

                    Message m = list.get(i);
                    log.debug("gateway id --> informer id: " +m.getProperty("id")+" --> "+informer_message_ids[i]);

                    try{
                        SubmitSMResp submitSMResp = new SubmitSMResp();
                        Properties p = m.getProperties();
                        long message_id = Long.parseLong(p.getProperty("id"));
                        submitSMResp.setSequenceNumber(id_seq_num_table.get(message_id));
                        submitSMResp.setConnectionName(id_conn_name_table.get(message_id));
                        submitSMResp.setMessageId(Long.toString(message_id));
                        submitSMResp.setStatus(Status.OK);
                        smppServer.send(submitSMResp, false);

                        id_seq_num_table.remove(message_id);
                        id_conn_name_table.remove(message_id);

                        log.debug("Send smpp response with status 'OK' for "+message_id+" message from "+delivery_id+" delivery." );
                    } catch (SmppException e) {
                        log.error("Could not send response to client", e);
                        // todo ?
                    }

                }

                queue.clear();
                log.debug("Clean queue for delivery with '"+delivery_id+"'.");

            } catch (AdminException e) {
                log.error("Couldn't add list with messages to delivery with id '"+delivery_id+"'.",e);

                for(Message m: list){

                    try{
                        SubmitSMResp submitSMResp = new SubmitSMResp();
                        submitSMResp.setStatus(Status.SYSERR);
                        Properties p = m.getProperties();
                        long id = Long.parseLong(p.getProperty("id"));
                        submitSMResp.setSequenceNumber(id_seq_num_table.get(id));
                        submitSMResp.setConnectionName(id_conn_name_table.get(id));
                        smppServer.send(submitSMResp, false);

                        id_seq_num_table.remove(id);
                        id_conn_name_table.remove(id);
                    } catch (SmppException e2) {
                        log.error("Could not send response to client", e2);
                        // todo ?
                    }
                }

                queue.clear();
                log.debug("Clean queue for delivery with '"+delivery_id+"'.");
            }

        } else {
            log.debug(delivery_id+"_queue is empty.");
        }

    }

    public void run() {
        log.debug("Start sender for connection: "+connection+".");
        while(!isInterrupted()){

            log.debug("Iterate queues for connection "+connection+".");

            while(!sendTaskQueue.isEmpty()){
                SendTask sendTask = sendTaskQueue.poll();
                sendTask.run();
            }

            synchronized (this){
                try{
                    log.debug("Sender for connection "+connection+" wait.");
                    wait();
                } catch (InterruptedException e) {
                    log.debug(e);
                    break;
                }
            }
        }
        connection.close();
        log.debug("Sender for connection "+connection+" finished work.");
    }

}

package mobi.eyeline.dcpgw.dcp;

import mobi.eyeline.dcpgw.Config;
import mobi.eyeline.dcpgw.exeptions.CouldNotWriteToJournalException;
import mobi.eyeline.dcpgw.journal.Journal;
import mobi.eyeline.dcpgw.smpp.Server;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.delivery.DeliveryState;
import mobi.eyeline.informer.admin.delivery.DeliveryStatistics;
import mobi.eyeline.informer.admin.delivery.protogen.DcpClient;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.*;
import mobi.eyeline.informer.util.Functions;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.pdu.SubmitSMResp;
import mobi.eyeline.smpp.api.types.Status;
import org.apache.log4j.Logger;

import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.*;

import static mobi.eyeline.informer.admin.delivery.DcpConverter.convert;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 25.09.11
 * Time: 15:22
 */
public class DcpConnectionImpl extends Thread implements DcpConnection{

    private static Logger log = Logger.getLogger(DcpConnectionImpl.class);

    private DcpClient client;

    private final ScheduledExecutorService scheduler = Executors.newSingleThreadScheduledExecutor();

    private final Map<Integer, LinkedBlockingQueue<Message>> delivery_id_queue_map;

    private Map<LinkedBlockingQueue<Message>, ScheduledFuture> queue_task_map;
    private LinkedBlockingQueue<SendTask> sendTaskQueue;

    private Hashtable<Long, SubmitSMResp> message_id_submit_sm_resp_table;

    private Calendar cal = Calendar.getInstance();

    private static SimpleDateFormat sdf = new SimpleDateFormat("yyMMddHHmmssSSS");

    private int capacity;
    private long timeout;

    private String informer_user;

    private static final TimeZone STAT_TIMEZONE=TimeZone.getTimeZone("UTC");
    private static final TimeZone LOCAL_TIMEZONE=TimeZone.getDefault();

    public DcpConnectionImpl(String informer_user) throws AdminException {
        super("ic_"+informer_user);
        this.informer_user = informer_user;

        Config config = Config.getInstance();

        this.client = new DcpClient(config.getInformerHost(), config.getInformerPort(),
                      informer_user, config.getInformerUserPassword(informer_user));

        delivery_id_queue_map = Collections.synchronizedMap(new HashMap<Integer, LinkedBlockingQueue<Message>>());
        queue_task_map = Collections.synchronizedMap(new HashMap<LinkedBlockingQueue<Message>, ScheduledFuture>());

        sendTaskQueue = new LinkedBlockingQueue<SendTask>();

        message_id_submit_sm_resp_table = new Hashtable<Long, SubmitSMResp>();

        capacity = config.getInformerMessagesListCapacity();
        timeout = config.getSendingToInformerTimeout();

        Set<Integer> deliveries = config.getDeliveries(informer_user);
        for(Integer delivery_id: deliveries){
            delivery_id_queue_map.put(delivery_id, new LinkedBlockingQueue<Message>(capacity));
            log.debug("Initialize "+delivery_id+"_queue.");
        }
        this.start();

        log.debug("Initialize connection for informer user "+informer_user+".");
    }

    public void addMessage(int delivery_id, Message informer_message,
                                        long message_id, SubmitSMResp resp) throws InterruptedException {
        LinkedBlockingQueue<Message> queue = delivery_id_queue_map.get(delivery_id);

        synchronized (queue){
            queue.put(informer_message);
            int size = queue.size();
            log.debug("add "+message_id+"_message to "+delivery_id+"_queue, size "+size);
            message_id_submit_sm_resp_table.put(message_id, resp);
            //log.debug("Remember SubmitSMResp for message_id "+message_id);

            if (queue.size() == 1){
                SendTask sendTask = new SendTask(delivery_id);
                ScheduledFuture scheduledFuture = scheduler.schedule(sendTask, timeout, TimeUnit.MILLISECONDS);
                queue_task_map.put(queue, scheduledFuture);
            }

            if (size == capacity) {
                log.debug(delivery_id+"_queue is full");
                ScheduledFuture scheduledFuture = queue_task_map.get(queue);
                if (scheduledFuture.cancel(false)){
                    log.debug("Cancel initial "+delivery_id+"_send_task.");
                } else {
                    log.debug("Couldn't cancel initial "+delivery_id+"_send_task, task has already started");
                }
                queue_task_map.remove(queue);
                log.debug("queue_scheduled_future map size "+queue_task_map.size());

                SendTask sendTask = new SendTask(delivery_id);
                sendTaskQueue.add(sendTask);
                log.debug("send_task_queue size "+sendTaskQueue.size());

                synchronized (this){
                    notifyAll();
                    log.debug("notified all waiting threads");
                }
            }
        }
    }

    public void sendMessages(int delivery_id){
        LinkedBlockingQueue<Message> queue = delivery_id_queue_map.get(delivery_id);

            long t = System.currentTimeMillis();

            if (!queue.isEmpty()){
                log.debug(delivery_id+"_queue is not empty and has size "+queue.size());

                List<Message> list = new ArrayList<Message>();
                queue.drainTo(list);

                try{
                    log.debug("Try to add list with messages to delivery with id '"+delivery_id+"' ...");


                    long[] informer_message_ids = addDeliveryMessages(delivery_id, list);

                    log.debug("Successfully add list with messages to delivery with id '"+delivery_id+"'.");

                    DeliveryStatistics delivery_statistics = getDeliveryState(delivery_id);
                    DeliveryState ds = delivery_statistics.getDeliveryState();

                    if (ds.getStatus() == mobi.eyeline.informer.admin.delivery.DeliveryStatus.Finished){
                        log.debug("Detected that delivery status is finished.");
                        DeliveryState deliveryState = new DeliveryState();
                        deliveryState.setStatus(mobi.eyeline.informer.admin.delivery.DeliveryStatus.Planned);
                        changeDeliveryState(delivery_id, deliveryState);
                        log.debug("Change delivery status on planned.");
                    }

                    for(int i = 0; i < list.size(); i++ ){

                        Message m = list.get(i);
                        Properties p = m.getProperties();
                        String message_id_str = p.getProperty("id");
                        long message_id = Long.parseLong(message_id_str);

                        log.debug("gateway id --> informer id: " +message_id_str+" --> "+informer_message_ids[i]);

                        try{
                            SubmitSMResp resp = message_id_submit_sm_resp_table.remove(message_id);
                            if (resp != null){
                                resp.setMessageId(message_id_str);
                                resp.setStatus(Status.OK);
                                Server.getInstance().send(resp);
                                log.debug("send SubmitSMResp: id="+message_id+", sn="+resp.getSequenceNumber()+", status=OK, delivery_id="+delivery_id);

                                long submit_time = System.currentTimeMillis();
                                Date submit_date = Functions.convertTime(new Date(submit_time), LOCAL_TIMEZONE, STAT_TIMEZONE);
                                try {
                                    Journal.getInstance().writeSubmitDate(message_id, submit_date, false);
                                } catch (CouldNotWriteToJournalException e) {
                                    log.error("Couldn't write submit date to journal.", e);
                                }

                            } else {
                                log.error("Couldn't find SubmitSMResp data, message_id="+message_id_str);
                            }
                        } catch (SmppException e) {
                            log.error("Could not send response to client", e);
                        }

                    }

                } catch (AdminException e) {
                    log.error("Couldn't add list with messages to delivery with id '"+delivery_id+"'.",e);

                    for(Message m: list){

                        try{

                            Properties p = m.getProperties();
                            long id = Long.parseLong(p.getProperty("id"));
                            SubmitSMResp resp = message_id_submit_sm_resp_table.remove(id);
                            resp.setStatus(Status.SYSERR);
                            Server.getInstance().send(resp);
                        } catch (SmppException e2) {
                            log.error("Could not send response to client", e2);
                        }

                    }

                }

            } else {
                log.debug(delivery_id+"_queue is empty.");
            }

            long dif = System.currentTimeMillis() - t;
            log.debug("Done send messages task, "+dif+" mls");
    }

    public void run() {
        log.debug("Start dcp connection for "+informer_user+".");
        while(!isInterrupted()){

            log.debug("send task queue size "+sendTaskQueue.size());
            while(!sendTaskQueue.isEmpty()){
                SendTask sendTask = sendTaskQueue.poll();
                sendTask.run();
            }

            synchronized (this){
                try{
                    log.debug("go into a wait state ...");
                    wait();
                    log.debug("exit from a wait state ");
                } catch (InterruptedException e) {
                    log.debug(e);
                    break;
                }
            }
        }
        close();
        log.debug("Close connection for informer user "+informer_user+".");
    }

    public void close() {
        client.close();
    }

    public long[] addDeliveryMessages(int deliveryId, List<Message> messages) throws AdminException {
        AddDeliveryMessages req = new AddDeliveryMessages();
        req.setDeliveryId(deliveryId);
        req.setMessages(convert(messages));
        AddDeliveryMessagesResp resp = client.send(req);
        return resp.getMessageIds();
    }

    public void changeDeliveryState(int deliveryId, DeliveryState state) throws AdminException {
        ChangeDeliveryState req = new ChangeDeliveryState();
        req.setDeliveryId(deliveryId);
        req.setState(convert(state));
        client.send(req);
    }

    public DeliveryStatistics getDeliveryState(int deliveryId) throws AdminException {
        GetDeliveryState req = new GetDeliveryState();
        req.setDeliveryId(deliveryId);
        GetDeliveryStateResp resp;
        resp = client.send(req);
        return convert(resp.getStats(), resp.getState());
    }

    public String toString(){
        return "DcpConnection{user="+informer_user+"}";
    }

    class SendTask implements Runnable{

        private final int delivery_id;

        public SendTask(int delivery_id){
            this.delivery_id = delivery_id;
        }

        public void run() {
            log.debug("Start send task for "+delivery_id+"_queue.");
            sendMessages(delivery_id);
            log.debug("Send task for "+delivery_id+"_queue has finished.");
        }

    }
}

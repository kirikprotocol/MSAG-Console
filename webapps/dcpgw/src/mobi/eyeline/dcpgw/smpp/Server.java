package mobi.eyeline.dcpgw.smpp;

import mobi.eyeline.dcpgw.Config;
import mobi.eyeline.dcpgw.exeptions.CouldNotLoadJournalException;
import mobi.eyeline.dcpgw.exeptions.CouldNotWriteToJournalException;
import mobi.eyeline.dcpgw.exeptions.InitializationException;
import mobi.eyeline.dcpgw.journal.Data;
import mobi.eyeline.dcpgw.journal.Journal;
import mobi.eyeline.smpp.api.*;
import mobi.eyeline.smpp.api.pdu.DeliverSM;
import mobi.eyeline.smpp.api.pdu.DeliverSMResp;
import mobi.eyeline.smpp.api.pdu.PDU;
import mobi.eyeline.smpp.api.pdu.data.Address;
import mobi.eyeline.smpp.api.types.EsmMessageType;
import org.apache.log4j.Logger;

import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.Executors;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 08.09.11
 * Time: 9:58
 */
public class Server{

    private static Logger log = Logger.getLogger(Server.class);

    private static final Object monitor = new Object();

    private static Server instance = new Server();

    private SubSmppServer server;

    private static SimpleDateFormat sdf = new SimpleDateFormat("yyMMddHHmmssSSS");
    private static SimpleDateFormat sdf2 = new SimpleDateFormat("ddHHmmss");
    private static Calendar cal = Calendar.getInstance();
    private static AtomicInteger ai = new AtomicInteger(0);

    private static Hashtable<Integer, Data> sequence_number_receipt_table;

    private int resend_receipts_timeout;
    private int resend_receipts_max_timeout;
    private int send_receipts_limit;

    private Hashtable<String, LinkedBlockingQueue<Data>> connection_data_queue_table;
    private Hashtable<String, LinkedBlockingQueue<Integer>> connection_sn_queue_table;

    public static Server getInstance(){
        return instance;
    }

    public void init(Properties config, PDUListener listener) throws SmppException, InitializationException {
        server = new SubSmppServer(config, listener);

        resend_receipts_timeout = Config.getInstance().getRecendReceiptsTimeout();
        resend_receipts_max_timeout = Config.getInstance().getRecendReceiptsMaxTimeout();
        send_receipts_limit = Config.getInstance().getSendReceiptLimit();

        try {
            sequence_number_receipt_table = Journal.getInstance().load();
        } catch (CouldNotLoadJournalException e) {
            log.error("Couldn't load journal.", e);
            throw new InitializationException(e);
        }

        ScheduledExecutorService resend_delivery_recepits_scheduler = Executors.newSingleThreadScheduledExecutor();
        resend_delivery_recepits_scheduler.scheduleWithFixedDelay(new Runnable() {

            @Override
            public void run() {
                resendDeliveryReceipts();
            }

        }, Config.getInstance().getRecendReceiptsInterval(), Config.getInstance().getRecendReceiptsInterval(), TimeUnit.SECONDS);

        ScheduledExecutorService send_delivery_recepits_scheduler = Executors.newSingleThreadScheduledExecutor();
        resend_delivery_recepits_scheduler.scheduleWithFixedDelay(new Runnable() {

            @Override
            public void run() {
                sendDeliveryReceipts();
            }

        }, Config.getInstance().getRecendReceiptsInterval(), Config.getInstance().getRecendReceiptsInterval(), TimeUnit.SECONDS);

        connection_data_queue_table = new Hashtable<String, LinkedBlockingQueue<Data>>();
        connection_sn_queue_table = new Hashtable<String, LinkedBlockingQueue<Integer>>();
    }

    public void send(PDU resp) throws SmppException {
        server.send(resp, false);
    }

    public void update(Properties config) throws SmppException {
        server.update(config);
    }

    public void shutdown(){
        server.shutdown();
    }

    private void sendDeliveryReceipts(){

        synchronized (monitor) {

        for(String connection_name: connection_sn_queue_table.keySet()){
            LinkedBlockingQueue<Integer> sn_queue = connection_sn_queue_table.get(connection_name);
            if (sn_queue.size() < send_receipts_limit){
                LinkedBlockingQueue<Data> queue = connection_data_queue_table.get(connection_name);
                int available = send_receipts_limit - sn_queue.size();
                for(int i=0; i<available; i++){
                    Data data = queue.poll();

                    long first_sending_time = System.currentTimeMillis();
                    data.setFirstSendingTime(first_sending_time);
                    data.setLastResendTime(first_sending_time);

                    long message_id = data.getMessageId();
                    int nsms = data.getNsms();
                    Date done_date = data.getDoneDate();
                    Date submit_date = data.getSubmitDate();
                    Address source_address = data.getSourceAddress();
                    Address destination_address = data.getDestinationAddress();
                    FinalMessageState state = data.getFinalMessageState();

                    String message = "id:" + message_id +
                    " sub:" + nsms + " dlvrd:" + nsms + " submit date:" + sdf.format(submit_date) +
                    " done date:" + sdf.format(done_date) +
                    " stat:" + state +
                    " err:000 Text:";
                    log.debug("Receipt message: " + message);

                    DeliverSM deliverSM = new DeliverSM();
                    deliverSM.setEsmMessageType(EsmMessageType.DeliveryReceipt);
                    deliverSM.setSourceAddress(destination_address);
                    deliverSM.setDestinationAddress(source_address);
                    deliverSM.setConnectionName(connection_name);
                    deliverSM.setMessage(message);

                    Date date = cal.getTime();
                    int sn = Integer.parseInt(sdf2.format(date)) + ai.incrementAndGet();
                    deliverSM.setSequenceNumber(sn);

                    data.setSequenceNumber(sn);

                    sequence_number_receipt_table.put(sn, data);
                    log.debug("remember data: " + sn + " --> " + data +", table size: "+sequence_number_receipt_table.size());

                    try {
                        server.send(deliverSM, false);
                        log.debug("send DeliverSM: sn=" + sn + ", id=" + data.getMessageId());

                        long send_receipt_time = System.currentTimeMillis();
                        data.setLastResendTime(send_receipt_time);
                        data.setStatus(Data.Status.SEND);

                        sequence_number_receipt_table.put(sn, data);
                        try {
                            Journal.getInstance().write(data);
                        } catch (CouldNotWriteToJournalException e) {
                            log.error(e);
                        }
                    } catch (SmppException e) {
                        log.warn(e);

                        long send_receipt_time = System.currentTimeMillis();
                        data.setLastResendTime(send_receipt_time);
                        data.setStatus(Data.Status.NOT_SEND);

                        sequence_number_receipt_table.put(sn, data);
                        try {
                            Journal.getInstance().write(data);
                        } catch (CouldNotWriteToJournalException e2) {
                            log.error(e2);
                        }
                    }

                }
            }
        }

        }




    }

    public void addDeliveryReceiptData(Data data){
        String connection_name = data.getConnectionName();
        // Create new queue if it doesn't exist.
        if (connection_data_queue_table.get(connection_name) == null){
            connection_data_queue_table.put(connection_name, new LinkedBlockingQueue<Data>());
        }

        LinkedBlockingQueue<Data> queue = connection_data_queue_table.get(connection_name);

        queue.add(data);

        try {
            Journal.getInstance().write(data);
        } catch (CouldNotWriteToJournalException e) {
            log.error(e);
        }

        log.debug("add "+data.getMessageId()+"_rcpt to "+connection_name+"_queue.");
    }

    public void resendDeliveryReceipts() {
        log.debug("Check receipts table to recent unanswered receipts ...");

        long current_time = System.currentTimeMillis();

        HashSet<Integer> timeout_expired_sequence_numbers = new HashSet<Integer>();
        HashSet<Integer> max_timeout_expired_sequence_numbers = new HashSet<Integer>();

        for (Map.Entry<Integer, Data> entry : sequence_number_receipt_table.entrySet()) {

            int sequence_number = entry.getKey();
            Data data = entry.getValue();
            long last_resend_time = data.getLastResendTime();
            long first_sending_time = data.getFirstSendingTime();


            if (current_time - first_sending_time < resend_receipts_max_timeout * 1000 * 60){

                if (current_time - last_resend_time >= resend_receipts_timeout * 1000)
                    timeout_expired_sequence_numbers.add(sequence_number);

            } else {
                    max_timeout_expired_sequence_numbers.add(sequence_number);
            }

        }

        for (Integer sn : max_timeout_expired_sequence_numbers) {
            synchronized (monitor) {
                Data data = sequence_number_receipt_table.remove(sn);
                log.debug("The maximum time of resending delivery receipt for message with id " + data.getMessageId() + " expired.");
                data.setStatus(Data.Status.EXPIRED_MAX_TIMEOUT);
                try {
                    Journal.getInstance().write(data);
                } catch (CouldNotWriteToJournalException e) {
                    log.error(e);
                }
                log.debug("Remove deliver receipt data with sequence number " + sn + " and message id " +
                        data.getMessageId() + " from memory and write to journal with status " + Data.Status.EXPIRED_MAX_TIMEOUT +
                        ", table size "+sequence_number_receipt_table.size()+".");
            }
        }

        for (Integer sn : timeout_expired_sequence_numbers) {
            if (!max_timeout_expired_sequence_numbers.contains(sn)) {
                synchronized (monitor) {
                    Data data = sequence_number_receipt_table.remove(sn);
                    log.warn("DeliverSM with sequence number " + sn + " expired. There was no DeliverSMResp within " + resend_receipts_timeout + " seconds. ");
                    data.setStatus(Data.Status.EXPIRED_TIMEOUT);

                    try {
                        Journal.getInstance().write(data);
                    } catch (CouldNotWriteToJournalException e) {
                        log.error(e);
                    }

                    DeliverSM deliverSM = new DeliverSM();
                    deliverSM.setEsmMessageType(EsmMessageType.DeliveryReceipt);
                    deliverSM.setSourceAddress(data.getSourceAddress());
                    deliverSM.setDestinationAddress(data.getDestinationAddress());
                    deliverSM.setConnectionName(data.getConnectionName());

                    Date date = cal.getTime();
                    int new_sn = Integer.parseInt(sdf2.format(date)) + ai.incrementAndGet();
                    deliverSM.setSequenceNumber(new_sn);

                    String message = "id:" + data.getMessageId() +
                            " sub:" + data.getNsms() + " dlvrd:" + data.getNsms() + " submit date:" + sdf.format(data.getSubmitDate()) +
                            " done date:" + sdf.format(data.getDoneDate()) +
                            " stat:" + data.getFinalMessageState() +
                            " err:000 Text:";
                    log.debug("Receipt message: " + message);
                    deliverSM.setMessage(message);


                    try {
                        server.send(deliverSM, false);
                        log.debug("resend DeliverSM: sn=" + new_sn + ", message_id=" + data.getMessageId());

                        long send_receipt_time = System.currentTimeMillis();
                        data.setLastResendTime(send_receipt_time);
                        data.setStatus(Data.Status.SEND);

                        sequence_number_receipt_table.put(new_sn, data);
                        log.debug("remember data: " + new_sn + " --> " + data +", table size: "+sequence_number_receipt_table.size());


                        try {
                            Journal.getInstance().write(data);
                        } catch (CouldNotWriteToJournalException e) {
                            log.error(e);
                        }
                    } catch (ConnectionNotFoundException e) {
                        log.warn(e);

                        long send_receipt_time = System.currentTimeMillis();
                        data.setLastResendTime(send_receipt_time);
                        data.setStatus(Data.Status.NOT_SEND);

                        sequence_number_receipt_table.put(new_sn, data);
                        try {
                            Journal.getInstance().write(data);
                        } catch (CouldNotWriteToJournalException e2) {
                            log.error(e2);
                        }
                    } catch (ConnectionNotEstablishedException e) {
                        log.warn(e);

                        long send_receipt_time = System.currentTimeMillis();
                        data.setLastResendTime(send_receipt_time);
                        data.setStatus(Data.Status.NOT_SEND);

                        sequence_number_receipt_table.put(new_sn, data);
                        try {
                            Journal.getInstance().write(data);
                        } catch (CouldNotWriteToJournalException e2) {
                            log.error(e2);
                        }
                    } catch (SmppException e) {
                        log.error(e);
                    }
                }
            }
        }

        log.debug("Done resend task.");
    }

    public boolean handleDeliverSMResp(DeliverSMResp resp){
        synchronized (monitor) {

            int sequence_number = resp.getSequenceNumber();
            mobi.eyeline.smpp.api.types.Status status = resp.getStatus();
            log.debug("receive DeliverSMResp: sn=" + sequence_number+", status="+status);

            if (status == mobi.eyeline.smpp.api.types.Status.OK){
                Data data = sequence_number_receipt_table.remove(sequence_number);
                if (data != null) {
                    log.debug("Remove from memory deliver receipt data with sequence number " + sequence_number + " .");

                    data.setStatus(Data.Status.DONE);
                    try {
                        Journal.getInstance().write(data);
                    } catch (CouldNotWriteToJournalException e) {
                        log.error(e);
                    }
                } else {
                    log.warn("Couldn't find deliver receipt data with sequence number " + sequence_number);
                }
            } else {
                // todo ?
            }

        }
        return true;
    }

    private class SubSmppServer extends SmppServer{

        public SubSmppServer(Properties config, PDUListener listener) throws SmppException {
            super(config, listener);
        }

        public void update(Properties config) throws SmppException {
            configure(config);
        }

    }

}

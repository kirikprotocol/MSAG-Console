package mobi.eyeline.dcpgw.smpp;

import mobi.eyeline.dcpgw.Config;
import mobi.eyeline.dcpgw.exeptions.CouldNotWriteToJournalException;
import mobi.eyeline.dcpgw.journal.DeliveryReceiptData;
import mobi.eyeline.dcpgw.journal.Journal;
import mobi.eyeline.dcpgw.journal.SubmitSMData;
import mobi.eyeline.dcpgw.model.Delivery;
import mobi.eyeline.dcpgw.model.Provider;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.pdu.DeliverSM;
import mobi.eyeline.smpp.api.pdu.DeliverSMResp;
import mobi.eyeline.smpp.api.pdu.data.Address;
import mobi.eyeline.smpp.api.types.EsmMessageType;
import org.apache.log4j.Logger;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.concurrent.Executors;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 07.10.11
 * Time: 14:34
 */
public class Connection {

    private static Logger log = Logger.getLogger(Server.class);

    private static final Object monitor = new Object();

    private String name;

    private Hashtable<Integer, DeliveryReceiptData> sn_data_table;

    private LinkedBlockingQueue<DeliveryReceiptData> queue;

    private Hashtable<Long, Date> message_id_date_table;

    private Journal journal = Journal.getInstance();

    private static SimpleDateFormat sdf = new SimpleDateFormat("yyMMddHHmm");

    private int response_timeout;
    private int send_receipt_max_time;
    private int send_receipts_speed;

    ScheduledExecutorService scheduler = Executors.newSingleThreadScheduledExecutor();
    ScheduledExecutorService resend_delivery_receipts_scheduler = Executors.newSingleThreadScheduledExecutor();

    public Connection(String name, int send_receipts_speed, int send_receipt_max_time_min){
        this.name = name;
        this.send_receipts_speed = send_receipts_speed;
        this.send_receipt_max_time = send_receipt_max_time_min;

        Config config = Config.getInstance();
        response_timeout = config.getDeliveryResponseTimeout();

        sn_data_table = Journal.getInstance().getDataTable(name);
        queue = Journal.getInstance().getDataQueue(name);
        message_id_date_table = Journal.getInstance().getSubmitDateTable(name);

        if (sn_data_table == null) sn_data_table = new Hashtable<Integer, DeliveryReceiptData>();
        if (queue == null) queue = new LinkedBlockingQueue<DeliveryReceiptData>();
        if (message_id_date_table == null)  message_id_date_table = new Hashtable<Long, Date>();

        scheduler.scheduleWithFixedDelay(new Runnable() {

            @Override
            public void run() {
                 send();
            }

        }, 1, 1, TimeUnit.SECONDS);

        int t = config.getResendReceiptsInterval();

        resend_delivery_receipts_scheduler.scheduleWithFixedDelay(new Runnable() {

            @Override
            public void run() {
                resend();
            }

        }, t , t, TimeUnit.SECONDS);
        log.debug("Initialize scheduler with resend interval " + t + " sec.");
    }

    void send(DeliveryReceiptData data){

        if (queue == null) queue = new LinkedBlockingQueue<DeliveryReceiptData>();

        long message_id = data.getMessageId();
        Date submit_date = message_id_date_table.get(message_id);
        if (submit_date != null){

            data.setSubmitDate(submit_date);

            try {
                SubmitSMData sdata = new SubmitSMData();
                sdata.setMessageId(message_id);
                sdata.setConnectionName(data.getConnectionName());
                sdata.setSubmitDate(new Date(System.currentTimeMillis()));
                sdata.setStatus(SubmitSMData.Status.RECEIVE_DELIVERY_RECEIPT);
                journal.write(sdata);
            } catch (CouldNotWriteToJournalException e) {
                log.error("Couldn't write to submit date journal.", e);
            }

            try{
                queue.add(data);
                log.debug("add "+data.getMessageId()+"_rcpt to "+name+"_delivery_queue, size "+queue.size());
            } catch (IllegalStateException e) {
                log.warn(name+"_queue full, couldn't add delivery receipt with message_id "+data.getMessageId());
            }

            try {
                journal.write(data);
            } catch (CouldNotWriteToJournalException e) {
                log.error(e);
            }
        } else {
            log.error("Couldn't find submit date for receipt with message id "+data.getMessageId());
        }
    }

    public void send(){

        if (queue.isEmpty()) {
            //log.debug(name+"_delivery_queue is empty.");
            return;
        } else {
            log.debug(name+"_delivery_queue size is "+queue.size()+", "+name+"_sn_data_table size is "+sn_data_table.size());
        }

        synchronized (monitor) {

            if (sn_data_table.size() < send_receipts_speed){

                int available = send_receipts_speed - sn_data_table.size();
                if (available > 0){
                    log.debug(name+"_con, available="+available);
                    for(int i=0; i < available; i++){

                        DeliveryReceiptData data = queue.poll();

                        if (data != null){

                            long message_id = data.getMessageId();

                            Provider provider = Config.getInstance().getProvider(name);
                            if (provider == null){
                                log.debug("Couldn't find provider for message_id "+message_id+" with connection "+name+".");
                                continue;
                            }

                            Address source_address = data.getSourceAddress();
                            Address destination_address = data.getDestinationAddress();
                            Delivery delivery = provider.getDelivery(destination_address.getAddress());
                            if (delivery == null){
                                log.debug("Couldn't find delivery for message_id "+message_id+" with source address "+destination_address.getAddress()+".");
                                continue;
                            }


                            int nsms = data.getNsms();
                            Date done_date = data.getDoneDate();
                            Date submit_date = data.getSubmitDate();


                            FinalMessageState state = data.getFinalMessageState();

                            String message = "id:" + message_id +
                                             " nsms:" + nsms +
                                             " submit date:" + sdf.format(submit_date) +
                                             " done date:" + sdf.format(done_date) +
                                             " stat:" + state;

                            log.debug("Receipt message: " + message);

                            DeliverSM deliverSM = new DeliverSM();
                            deliverSM.setEsmMessageType(EsmMessageType.DeliveryReceipt);
                            deliverSM.setSourceAddress(source_address);
                            deliverSM.setDestinationAddress(destination_address);
                            deliverSM.setConnectionName(name);
                            deliverSM.setMessage(message);

                            int sn = Server.getInstance().getReceiptSequenceNumber();
                            deliverSM.setSequenceNumber(sn);

                            data.setSequenceNumber(sn);

                            try {
                                Server.getInstance().send(deliverSM, false);
                                log.debug("send DeliverSM: sn=" + sn + ", id=" + data.getMessageId());

                                long first_sending_time = System.currentTimeMillis();
                                data.setFirstSendingTime(first_sending_time);
                                data.setLastResendTime(first_sending_time);

                                data.setStatus(DeliveryReceiptData.Status.SEND);
                                sn_data_table.put(sn, data);

                                try {
                                    Journal.getInstance().write(data);
                                } catch (CouldNotWriteToJournalException e) {
                                    log.error(e);
                                }
                            } catch (SmppException e) {
                                log.warn(e);

                                long first_sending_time = System.currentTimeMillis();
                                data.setFirstSendingTime(first_sending_time);
                                data.setLastResendTime(first_sending_time);

                                data.setStatus(DeliveryReceiptData.Status.NOT_SEND);
                                sn_data_table.put(sn, data);

                                try {
                                    Journal.getInstance().write(data);
                                } catch (CouldNotWriteToJournalException e2) {
                                    log.error(e2);
                                }
                            }
                        } else {
                            break;
                        }

                    }
                }
            }
        }

    }

    public void resend() {
        //log.debug("Check "+name+"_sn_data table to resend unanswered receipts ...");
        synchronized (monitor){

            long current_time = System.currentTimeMillis();

            HashSet<Integer> timeout_expired_sequence_numbers = new HashSet<Integer>();
            HashSet<Integer> max_timeout_expired_sequence_numbers = new HashSet<Integer>();

            for(Integer sn: sn_data_table.keySet()){


                DeliveryReceiptData data = sn_data_table.get(sn);

                long message_id = data.getMessageId();
                Provider provider = Config.getInstance().getProvider(name);
                if (provider == null){
                    log.debug("Couldn't find provider for message_id "+message_id+" with connection "+name+".");
                    continue;
                }

                Address destination_address = data.getDestinationAddress();
                Delivery delivery = provider.getDelivery(destination_address.getAddress());
                if (delivery == null){
                    log.debug("Couldn't find delivery for message_id "+message_id+" with source address "+destination_address.getAddress()+".");
                    continue;
                }

                long last_resend_time = data.getLastResendTime();

                long init_time = data.getInitTime();

                long dif1 = current_time - init_time;
                long dif2 = current_time - last_resend_time;

                if (dif1 < send_receipt_max_time * 1000 * 60){

                    if (dif2 >= response_timeout * 1000) {

                        timeout_expired_sequence_numbers.add(sn);
                        log.debug(name+"_connection: "+sn+"_data expired, remember it");
                    } else {

                        log.debug(name+"_connection: "+sn+"_data doesn't expired, dif "+dif2/1000+" sec less than "+response_timeout+" sec.");
                    }

                } else {

                    max_timeout_expired_sequence_numbers.add(sn);
                    log.debug(name+"_connection: "+sn+"_data expired max time, remember it");
                }

            }

            for (Integer sn : max_timeout_expired_sequence_numbers) {

                DeliveryReceiptData data = sn_data_table.remove(sn);
                log.debug(name+"_connection: remove " + data.getMessageId() + "_message from "+name+"sn_data_table");
                data.setStatus(DeliveryReceiptData.Status.EXPIRED_MAX_TIMEOUT);
                try {
                    Journal.getInstance().write(data);
                } catch (CouldNotWriteToJournalException e) {
                    log.error(e);
                }
                log.debug(name+"_connection: Remove deliver receipt data with sequence number " + sn + " and message id " +
                    data.getMessageId() + " from memory and write to journal with status " + DeliveryReceiptData.Status.EXPIRED_MAX_TIMEOUT +
                    ", table size "+sn_data_table.size()+".");

            }

            for (Integer sn : timeout_expired_sequence_numbers) {
                if (!max_timeout_expired_sequence_numbers.contains(sn)) {

                    DeliveryReceiptData data = sn_data_table.remove(sn);
                    log.warn(name+"_connection: DeliverSM with sequence number " + sn + " expired. There was no DeliverSMResp within " + response_timeout + " seconds. ");
                    data.setStatus(DeliveryReceiptData.Status.EXPIRED_TIMEOUT);

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

                    int new_sn = Server.getInstance().getReceiptSequenceNumber();
                    deliverSM.setSequenceNumber(new_sn);

                    String message = "id:" + data.getMessageId() +
                                    " dlvrd:" + data.getNsms() +
                                    " submit date:" + sdf.format(data.getSubmitDate()) +
                                    " done date:" + sdf.format(data.getDoneDate()) +
                                    " stat:" + data.getFinalMessageState();

                    log.debug(name+"_connection: Receipt "+data.getMessageId()+"_message: " + message);
                    deliverSM.setMessage(message);

                    try {
                        Server.getInstance().send(deliverSM, false);
                        log.debug(name+"_connection: resend DeliverSM: sn=" + new_sn + ", message_id=" + data.getMessageId());

                        long send_receipt_time = System.currentTimeMillis();
                        data.setLastResendTime(send_receipt_time);
                        data.setStatus(DeliveryReceiptData.Status.SEND);

                        sn_data_table.put(new_sn, data);
                        log.debug(name+"_connection: remember data: " + new_sn + " --> " + data +", table size: "+sn_data_table.size());

                        try {
                            Journal.getInstance().write(data);
                        } catch (CouldNotWriteToJournalException e) {
                                log.error(e);
                        }
                    } catch (SmppException e) {
                        log.warn(e);

                        long send_receipt_time = System.currentTimeMillis();
                        data.setLastResendTime(send_receipt_time);
                        data.setStatus(DeliveryReceiptData.Status.NOT_SEND);

                        sn_data_table.put(new_sn, data);
                        try {
                            Journal.getInstance().write(data);
                        } catch (CouldNotWriteToJournalException e2) {
                            log.error(e2);
                        }
                    }

                }

            }

            // Remove expired data from queue.
            for (DeliveryReceiptData data : queue) {
                long init_time = data.getInitTime();

                long dif = current_time - init_time;
                if (dif > send_receipt_max_time * 1000 * 60) {
                    log.debug(name+"_connection: "+data.getMessageId()+"_queue_data expired max time");
                    queue.remove(data);
                    log.debug(name+"_connection: remove " + data.getMessageId() + "_queue_data from " + name + "_queue.");

                    data.setStatus(DeliveryReceiptData.Status.DELETED);
                    try {
                        journal.write(data);
                    } catch (CouldNotWriteToJournalException e) {
                        log.error("Couldn't write to journal "+data.getMessageId()+"data.", e);
                    }

                } else {
                    log.debug(name+"_connection: "+data.getMessageId()+"_queue_data doesn't expired max time, dif "+dif/(1000*60) +" min less than "+send_receipt_max_time+" min.");
                }

            }

            //log.debug("Done "+name+"_resend_task");
        }

    }


    public boolean handle(DeliverSMResp resp){

        synchronized (monitor) {

            int sequence_number = resp.getSequenceNumber();
            String connection = resp.getConnectionName();
            mobi.eyeline.smpp.api.types.Status status = resp.getStatus();
            log.debug("receive DeliverSMResp: con="+connection+", sn=" + sequence_number+", status="+status);

            if (status == mobi.eyeline.smpp.api.types.Status.OK){
                DeliveryReceiptData data = sn_data_table.remove(sequence_number);
                if (data != null) {
                    log.debug("Remove from memory deliver receipt data with sequence number " + sequence_number + " .");

                    data.setStatus(DeliveryReceiptData.Status.DONE);
                    try {
                        Journal.getInstance().write(data);
                    } catch (CouldNotWriteToJournalException e) {
                        log.error(e);
                    }
                } else {
                    log.warn("Couldn't find deliver receipt data with sequence number " + sequence_number);
                }
            } else if (status == mobi.eyeline.smpp.api.types.Status.RX_P_APPN) {
                DeliveryReceiptData data = sn_data_table.remove(sequence_number);
                if (data != null) {
                    log.debug("Remove from memory deliver receipt data with sequence number " + sequence_number + " .");

                    data.setStatus(DeliveryReceiptData.Status.PERM_ERROR);
                    try {
                        Journal.getInstance().write(data);
                    } catch (CouldNotWriteToJournalException e) {
                        log.error(e);
                    }
                } else {
                    log.warn("Couldn't find deliver receipt data with sequence number " + sequence_number);
                }
            } else if (status == mobi.eyeline.smpp.api.types.Status.RX_T_APPN) {
                log.debug("Handle DeliverSMResp with status "+status);
            } else {
                log.error("Handle DeliverSMResp with unforeseen status "+status);
            }

        }

        return true;
    }

    public void close(){

        while(!queue.isEmpty()){
            DeliveryReceiptData data = queue.poll();
            data.setStatus(DeliveryReceiptData.Status.DELETED);
            try {
                Journal.getInstance().write(data);
            } catch (CouldNotWriteToJournalException e) {
                log.error(e);
            }
        }

        for(Integer sn: sn_data_table.keySet()){
            DeliveryReceiptData data = sn_data_table.get(sn);
            data.setStatus(DeliveryReceiptData.Status.DELETED);
            try {
                Journal.getInstance().write(data);
            } catch (CouldNotWriteToJournalException e) {
                log.error(e);
            }
        }

        sn_data_table.clear();
    }

    public void setSendReceiptsSpeed(int send_receipts_speed){
        log.debug(name+"_connection: set speed "+send_receipts_speed);
        this.send_receipts_speed = send_receipts_speed;
    }

    public void setSendReceiptMaxTimeout(int send_receipt_max_time){
        log.debug(name+"_connection: set max time "+send_receipt_max_time);
        this.send_receipt_max_time = send_receipt_max_time;
    }

    public void setSubmitDate(long message_id, Date date){
        message_id_date_table.put(message_id, date);
    }

}

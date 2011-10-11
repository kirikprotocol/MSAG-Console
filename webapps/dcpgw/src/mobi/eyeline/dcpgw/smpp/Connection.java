package mobi.eyeline.dcpgw.smpp;

import mobi.eyeline.dcpgw.Config;
import mobi.eyeline.dcpgw.exeptions.CouldNotWriteToJournalException;
import mobi.eyeline.dcpgw.journal.Data;
import mobi.eyeline.dcpgw.journal.Journal;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.pdu.DeliverSM;
import mobi.eyeline.smpp.api.pdu.DeliverSMResp;
import mobi.eyeline.smpp.api.pdu.data.Address;
import mobi.eyeline.smpp.api.types.EsmMessageType;
import org.apache.log4j.Logger;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.concurrent.Executors;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

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

    private Hashtable<Integer, Data> sn_data_table;

    private LinkedBlockingQueue<Data> queue;

    private Config config = Config.getInstance();

    private Journal journal = Journal.getInstance();

    private static SimpleDateFormat sdf = new SimpleDateFormat("yyMMddHHmmssSSS");

    private static SimpleDateFormat sdf2 = new SimpleDateFormat("ddHHmmss");
    private static Calendar cal = Calendar.getInstance();
    private static AtomicInteger ai = new AtomicInteger(0);

    private int response_timeout;
    private int response_max_timeout;
    private int request_limit;

    ScheduledExecutorService scheduler = Executors.newSingleThreadScheduledExecutor();
    ScheduledExecutorService resend_delivery_receipts_scheduler = Executors.newSingleThreadScheduledExecutor();

    public Connection(String name){
        this.name = name;

        response_timeout = config.getDeliveryResponseTimeout();
        response_max_timeout = config.getDeliveryResponseMaxTimeout();
        request_limit = config.getDeliveryRequestLimit();

        sn_data_table = Journal.getInstance().getDataTable(name);
        queue = Journal.getInstance().getDataQueue(name);

        if (sn_data_table == null) sn_data_table = new Hashtable<Integer, Data>();
        if (queue == null) queue = new LinkedBlockingQueue<Data>(config.getDeliveryQueueLimit());

        int t1 = config.getSendReceiptsInterval();

        scheduler.scheduleWithFixedDelay(new Runnable() {

            @Override
            public void run() {
                 send();
            }

        }, t1, t1, TimeUnit.MILLISECONDS);
        log.debug("Initialize scheduler with send interval "+t1+" mls.");

        int t2 = config.getResendReceiptsInterval();

        resend_delivery_receipts_scheduler.scheduleWithFixedDelay(new Runnable() {

            @Override
            public void run() {
                resend();
            }

        }, t2 , t2, TimeUnit.SECONDS);
        log.debug("Initialize scheduler with resend interval " + t2 + " sec.");
    }

    void send(Data data){

        if (queue == null) queue = new LinkedBlockingQueue<Data>(config.getDeliveryQueueLimit());

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
    }

    public void send(){

        if (queue.isEmpty()) {
            //log.debug(name+"_delivery_queue is empty.");
            return;
        } else {
            log.debug(name+"_delivery_queue size is "+queue.size()+", "+name+"_sn_data_table size is "+sn_data_table.size());
        }

        synchronized (monitor) {

            if (sn_data_table.size() < request_limit){

                int available = request_limit - sn_data_table.size();
                log.debug(name+"_con, available="+available);
                for(int i=0; i < available; i++){

                    Data data = queue.poll();

                    if (data != null){

                        long first_sending_time = System.currentTimeMillis();
                        data.setFirstSendingTime(first_sending_time);

                        long message_id = data.getMessageId();
                        int nsms = data.getNsms();
                        Date done_date = data.getDoneDate();
                        Date submit_date = data.getSubmitDate();
                        Address source_address = data.getSourceAddress();
                        Address destination_address = data.getDestinationAddress();
                        FinalMessageState state = data.getFinalMessageState();

                        String message = "id:" + message_id +
                                         " sub:" + nsms +
                                         " dlvrd:" + nsms +
                                         " submit date:" + sdf.format(submit_date) +
                                         " done date:" + sdf.format(done_date) +
                                         " stat:" + state +
                                         " err:000 Text:";

                        log.debug("Receipt message: " + message);

                        DeliverSM deliverSM = new DeliverSM();
                        deliverSM.setEsmMessageType(EsmMessageType.DeliveryReceipt);
                        deliverSM.setSourceAddress(destination_address);
                        deliverSM.setDestinationAddress(source_address);
                        deliverSM.setConnectionName(name);
                        deliverSM.setMessage(message);

                        Date date = cal.getTime();
                        int sn = Integer.parseInt(sdf2.format(date)) + ai.incrementAndGet();
                        deliverSM.setSequenceNumber(sn);

                        data.setSequenceNumber(sn);

                        try {
                            Server.getInstance().send(deliverSM);
                            log.debug("send DeliverSM: sn=" + sn + ", id=" + data.getMessageId());
                            data.setStatus(Data.Status.SEND);
                            sn_data_table.put(sn, data);

                            try {
                                Journal.getInstance().write(data);
                            } catch (CouldNotWriteToJournalException e) {
                                log.error(e);
                            }
                        } catch (SmppException e) {
                            log.warn(e);

                            data.setStatus(Data.Status.NOT_SEND);
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

    public void resend() {
        log.debug("Check receipts table to resend unanswered receipts ...");

        long current_time = System.currentTimeMillis();

        HashSet<Integer> timeout_expired_sequence_numbers = new HashSet<Integer>();
        HashSet<Integer> max_timeout_expired_sequence_numbers = new HashSet<Integer>();

        for(Integer sn: sn_data_table.keySet()){

            Data data = sn_data_table.get(sn);
            long last_resend_time = data.getLastResendTime();
            long first_sending_time = data.getFirstSendingTime();

            if (current_time - first_sending_time < response_max_timeout * 1000 * 60){

                if (current_time - last_resend_time >= response_timeout * 1000)
                    timeout_expired_sequence_numbers.add(sn);
                } else {
                    max_timeout_expired_sequence_numbers.add(sn);
                }

            }

            for (Integer sn : max_timeout_expired_sequence_numbers) {
                synchronized (monitor) {
                    Data data = sn_data_table.remove(sn);
                    log.debug("The maximum time of resending delivery receipt for message with id " + data.getMessageId() + " expired.");
                    data.setStatus(Data.Status.EXPIRED_MAX_TIMEOUT);
                    try {
                        Journal.getInstance().write(data);
                    } catch (CouldNotWriteToJournalException e) {
                        log.error(e);
                    }
                    log.debug("Remove deliver receipt data with sequence number " + sn + " and message id " +
                            data.getMessageId() + " from memory and write to journal with status " + Data.Status.EXPIRED_MAX_TIMEOUT +
                            ", table size "+sn_data_table.size()+".");
                }
            }

            for (Integer sn : timeout_expired_sequence_numbers) {
                if (!max_timeout_expired_sequence_numbers.contains(sn)) {
                    synchronized (monitor) {
                        Data data = sn_data_table.remove(sn);
                        log.warn("DeliverSM with sequence number " + sn + " expired. There was no DeliverSMResp within " + response_timeout + " seconds. ");
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
                            Server.getInstance().send(deliverSM);
                            log.debug("resend DeliverSM: sn=" + new_sn + ", message_id=" + data.getMessageId());

                            long send_receipt_time = System.currentTimeMillis();
                            data.setLastResendTime(send_receipt_time);
                            data.setStatus(Data.Status.SEND);

                            sn_data_table.put(new_sn, data);
                            log.debug("remember data: " + new_sn + " --> " + data +", table size: "+sn_data_table.size());

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

                            sn_data_table.put(new_sn, data);
                            try {
                                Journal.getInstance().write(data);
                            } catch (CouldNotWriteToJournalException e2) {
                                log.error(e2);
                            }
                        }

                    }
                }

        }

        log.debug("Done resend task.");
    }


    public boolean handle(DeliverSMResp resp){

        synchronized (monitor) {

            int sequence_number = resp.getSequenceNumber();
            String connection = resp.getConnectionName();
            mobi.eyeline.smpp.api.types.Status status = resp.getStatus();
            log.debug("receive DeliverSMResp: con="+connection+", sn=" + sequence_number+", status="+status);

            if (status == mobi.eyeline.smpp.api.types.Status.OK){
                Data data = sn_data_table.remove(sequence_number);
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

    public void close(){

        while(!queue.isEmpty()){
            Data data = queue.poll();
            data.setStatus(Data.Status.DELETED);
            try {
                Journal.getInstance().write(data);
            } catch (CouldNotWriteToJournalException e) {
                log.error(e);
            }
        }

        for(Integer sn: sn_data_table.keySet()){
            Data data = sn_data_table.get(sn);
            data.setStatus(Data.Status.DELETED);
            try {
                Journal.getInstance().write(data);
            } catch (CouldNotWriteToJournalException e) {
                log.error(e);
            }
        }

        sn_data_table.clear();
    }

}

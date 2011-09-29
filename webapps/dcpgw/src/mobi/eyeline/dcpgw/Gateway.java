package mobi.eyeline.dcpgw;

import mobi.eyeline.dcpgw.admin.protogen.protocol.UpdateConfigResp;
import mobi.eyeline.dcpgw.dcp.DcpConnection;
import mobi.eyeline.dcpgw.dcp.DcpConnectionImpl;
import mobi.eyeline.dcpgw.exeptions.CouldNotLoadJournalException;
import mobi.eyeline.dcpgw.exeptions.CouldNotReadMessageStateException;
import mobi.eyeline.dcpgw.exeptions.CouldNotWriteToJournalException;
import mobi.eyeline.dcpgw.exeptions.InitializationException;
import mobi.eyeline.dcpgw.journal.Data;
import mobi.eyeline.dcpgw.journal.Journal;
import mobi.eyeline.dcpgw.journal.Status;
import mobi.eyeline.dcpgw.model.Delivery;
import mobi.eyeline.dcpgw.model.Provider;
import mobi.eyeline.dcpgw.utils.Utils;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.delivery.MessageState;
import mobi.eyeline.informer.admin.delivery.changelog.ChangeDeliveryStatusEvent;
import mobi.eyeline.informer.admin.delivery.changelog.ChangeMessageStateEvent;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangeListener;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangesDetectorImpl;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.util.config.XmlConfigException;
import mobi.eyeline.protogen.framework.*;
import mobi.eyeline.smpp.api.ConnectionNotEstablishedException;
import mobi.eyeline.smpp.api.ConnectionNotFoundException;
import mobi.eyeline.smpp.api.PDUListener;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.pdu.*;
import mobi.eyeline.smpp.api.pdu.PDU;
import mobi.eyeline.smpp.api.pdu.data.Address;
import mobi.eyeline.smpp.api.pdu.data.InvalidAddressFormatException;
import mobi.eyeline.smpp.api.pdu.tlv.TLVString;
import mobi.eyeline.smpp.api.processing.ProcessingQueue;
import mobi.eyeline.smpp.api.processing.QueueException;
import mobi.eyeline.smpp.api.types.EsmMessageType;
import mobi.eyeline.smpp.api.types.RegDeliveryReceipt;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

public class Gateway extends Thread implements PDUListener {

    private static Logger log = Logger.getLogger(Gateway.class);

    private static final Object read_write_monitor = new Object();

    private static ConfigurableInRuntimeSmppServer smppServer;

    private ProcessingQueue procQueue;

    protected DeliveryChangesDetectorImpl deliveryChangesDetector;

    protected FileSystem fileSystem;

    private static Journal journal;

    private static Hashtable<Integer, Data> sequence_number_receipt_table;

    private static int recend_receipts_timeout;
    private static int recend_receipts_max_timeout;

    private static SimpleDateFormat sdf = new SimpleDateFormat("yyMMddHHmmssSSS");
    private static SimpleDateFormat sdf2 = new SimpleDateFormat("ddHHmmss");
    private static Calendar cal = Calendar.getInstance();
    private static AtomicInteger ai = new AtomicInteger(0);

    private static int capacity;

    private static long sending_timeout;

    private static Hashtable<String, Sender> user_sender_table;

    private static String informer_host;
    private static int informer_port;

    private ConfigurationManager cm = ConfigurationManager.getInstance();

    public static void main(String args[]) {
        String user_dir = System.getProperty("user.dir");
        log.debug("user dir:"+user_dir);
        String config_file = user_dir + File.separator + "conf" + File.separator + "config.properties";
        log.debug("config file: "+config_file);

        try {
            new Gateway(config_file);
        } catch (Exception e) {
            log.error(e);
        }

    }

    public Gateway(String config_file) throws InitializationException, SmppException, IOException, XmlConfigException {
        log.debug("Try to initialize gateway ...");
        Runtime.getRuntime().addShutdownHook(this);
        cm.init(config_file);
        Properties config = cm.getProperties();

        String s = config.getProperty("informer.host");
        if (s != null && !s.isEmpty()){
            informer_host = s;
            log.debug("Set informer host: "+ informer_host);
        } else {
            log.error("informer.host property is invalid or not specified in config");
            throw new InitializationException("informer.host property is invalid or not specified in config");
        }

        s = config.getProperty("informer.port");
        if (s != null && !s.isEmpty()){
            informer_port = Integer.parseInt(s);
            log.debug("Set informer port: "+ informer_port);
        } else {
            log.error("informer.port property is invalid or not specified in config");
            throw new InitializationException("informer.port property is invalid or not specified in config");
        }

        s = config.getProperty("informer.messages.list.capacity");
        if (s != null && !s.isEmpty()){
            capacity = Integer.parseInt(s);
            log.debug("Configuration property: informer.messages.list.capacity="+capacity);
        } else {
            log.error("Configuration property 'informer.messages.list.capacity' is invalid or not specified in config");
            throw new InitializationException("Configuration property 'informer.messages.list.capacity' is invalid or not specified in config");
        }

        s = config.getProperty("sending.timeout.mls");
        if (s != null && !s.isEmpty()){
            sending_timeout = Integer.parseInt(s);
            log.debug("Configuration property: sending.timeout.mls="+sending_timeout);
        } else {
            log.error("Configuration property 'sending.timeout.mls' is invalid or not specified in config");
            throw new InitializationException("Configuration property 'sending.timeout.mls' is invalid or not specified in config");
        }

        user_sender_table = new Hashtable<String, Sender>();

        String update_config_server_host = config.getProperty("update.config.server.host");

        s = Utils.getProperty(config, "update.config.server.port");
        int update_config_server_port = Integer.parseInt(s);

        try {
            new UpdateConfigServer(update_config_server_host, update_config_server_port);
        } catch (IOException e) {
            log.error("Couldn't initialize update config server socket with host '" + update_config_server_host + "' and port '" + update_config_server_port + "'. ", e);
            throw new InitializationException(e);
        }

        int max_journal_size_mb = Utils.getProperty(config, "max.journal.size.mb", 10);
        String journal_dir = Utils.getProperty(config, "journal.dir", System.getProperty("user.dir")+File.separator+"journal");
        journal = new Journal(new File(journal_dir), max_journal_size_mb);

        try {
            sequence_number_receipt_table = journal.load();
        } catch (CouldNotLoadJournalException e) {
            log.error("Couldn't load journal.", e);
            throw new InitializationException(e);
        }

        PDUListenerImpl pduListener = new PDUListenerImpl();
        procQueue = new ProcessingQueue(config, pduListener, null);

        smppServer = new ConfigurableInRuntimeSmppServer(config, this);

        for(String user : cm.getInformerUsers()){
            try {
                DcpConnection dcpConnection =
                        new DcpConnectionImpl(informer_host, informer_port, user);
                Sender sender = new Sender(dcpConnection, capacity, sending_timeout);
                sender.setSmppServer(smppServer);
                sender.start();
                user_sender_table.put(user, sender);
            } catch (AdminException e) {
                log.warn(e);
            }
        }

        String final_log_dir = Utils.getProperty(config, "final.log.dir", System.getProperty("user.dir") + File.separator + "final_log");

        fileSystem = FileSystem.getFSForSingleInst();
        try {
            deliveryChangesDetector = new DeliveryChangesDetectorImpl(new File(final_log_dir), fileSystem);

            DeliveryChangeListenerImpl deliveryChangeListener = new DeliveryChangeListenerImpl();
            deliveryChangesDetector.addListener(deliveryChangeListener);

            deliveryChangesDetector.start();
        } catch (InitException e) {
            log.error(e);
            throw new InitializationException(e);
        }

        int recend_receipts_interval = Utils.getProperty(config, "resend.receipts.interval.sec", 60);

        recend_receipts_timeout = Utils.getProperty(config, "resend.receipts.timeout.sec", 60);

        recend_receipts_max_timeout = Utils.getProperty(config, "resend.receipts.max.timeout.min", 720);

        ScheduledExecutorService resend_delivery_recepits_scheduler = Executors.newSingleThreadScheduledExecutor();
        resend_delivery_recepits_scheduler.scheduleWithFixedDelay(new Runnable() {

            @Override
            public void run() {
                resendDeliveryReceipts();
            }

        }, recend_receipts_interval, recend_receipts_interval, TimeUnit.SECONDS);

        long clean_journal_timeout = Utils.getProperty(config, "clean.journal.timeout.msl", 60000);

        ScheduledExecutorService clean_journal_scheduler = Executors.newSingleThreadScheduledExecutor();
        clean_journal_scheduler.scheduleWithFixedDelay(new Runnable() {

            @Override
            public void run() {
                synchronized (read_write_monitor) {
                    journal.clean();
                }
            }

        }, clean_journal_timeout, clean_journal_timeout, TimeUnit.MILLISECONDS);

        log.debug("Gateway initialized.");
    }

    public void setDcpConnection(String user, DcpConnection dcpConnection){
        Sender sender = new Sender(dcpConnection, capacity, sending_timeout);
        user_sender_table.put(user, sender);
    }



    private synchronized void updateConfiguration() throws XmlConfigException, IOException, SmppException, AdminException {
        log.debug("Try to update configuration ...");

        Set<String> old_informer_users = cm.getInformerUsers();

        cm.update();

        // Update smpp connections
        smppServer.update(cm.getProperties());
        Set<String> new_informer_users = cm.getInformerUsers();

        // Add new dcp connections
        for(String new_user: new_informer_users){
            if (!old_informer_users.contains(new_user)){
                DcpConnection dcpConnection = new DcpConnectionImpl(informer_host, informer_port, new_user);

                Sender sender = new Sender(dcpConnection, capacity, sending_timeout);
                sender.setSmppServer(smppServer);
                sender.start();
                user_sender_table.put(new_user,sender);
            }
        }

        // Remove deleted dcp connections
        for(String old_user: user_sender_table.keySet()){
            if (!new_informer_users.contains(old_user)){
                Sender sender = user_sender_table.remove(old_user);
                sender.interrupt();
            }
        }

        log.debug("Configuration updated.");
    }

    public boolean handlePDU(PDU pdu) {
        try {
            procQueue.add(pdu, pdu.isMessage());
        } catch (QueueException e) {
            try {
                smppServer.send(((Request) pdu).getResponse(e.getStatus()), false);
            } catch (SmppException e1) {
                log.error("Could not send resp", e1);
            }
        }
        return true;
    }

    @Override
    public void run() {
        if (smppServer != null) smppServer.shutdown();
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Handle pdu

    private class PDUListenerImpl implements PDUListener{

        private AtomicLong al = new AtomicLong(0);

        @Override
        public boolean handlePDU(PDU pdu) {
            log.debug("Handle pdu with type "+pdu.getType());

            switch (pdu.getType()) {
                case SubmitSM:{

                    long time = System.currentTimeMillis();
                    long message_id = time + al.incrementAndGet();

                        Message request = (Message) pdu;
                        String connection_name = request.getConnectionName();

                        int sequence_number = request.getSequenceNumber();

                        log.debug("Handle pdu with type '"+pdu.getType()+"', sequence_number '"+sequence_number+"', connection_name '"+connection_name+"', set id '"+message_id+"'.");

                        Provider provider = ConfigurationManager.getInstance().getProvider(connection_name);
                        log.debug("This connection name corresponds to the provider with name '"+provider.getName()+"'.");

                        Address source_address = request.getSourceAddress();
                        String service_number = source_address.getAddress();

                        Address smpp_destination_address = request.getDestinationAddress();
                        String destination_address_str = smpp_destination_address.getAddress();

                        String text = request.getMessage();
                        log.debug("Message: "+text);
                        log.debug("Id '"+message_id+"', service number '"+service_number+"', destination address '"+destination_address_str+"', text '"+text+"'.");

                        Delivery delivery = provider.getDelivery(service_number);
                        if (delivery != null){

                            int delivery_id = delivery.getId();
                            String login= delivery.getUser();

                            log.debug("Message id '"+message_id+"', service number '"+service_number+"', delivery id '"+delivery_id+"', user '"+login+"'.");

                            Sender sender = user_sender_table.get(login);
                            sender.addMessage(message_id, service_number,
                                    destination_address_str, text, sequence_number, connection_name, delivery_id);

                            if (request.getRegDeliveryReceipt() == RegDeliveryReceipt.None )
                                log.warn("Received SubmitSM with RegDeliveryReceipt.None .");

                        } else {
                            log.debug("Provider "+provider.getName()+" doesn't have delivery with service number '"+service_number+"'.");
                            SubmitSMResp submitSMResp = new SubmitSMResp();

                            submitSMResp.setStatus(mobi.eyeline.smpp.api.types.Status.SUBMITFAIL);
                            submitSMResp.setSequenceNumber(sequence_number);
                            submitSMResp.setConnectionName(connection_name);
                            submitSMResp.setTLV(new TLVString( (short)0x001D, "Provider "+provider.getName()+" doesn't have delivery with service number '"+service_number+"'.") );
                            try {
                                smppServer.send(submitSMResp, false);
                                log.debug("SUBMIT_SM_RESP sn: "+sequence_number+", con: "+connection_name+", status '"+submitSMResp.getStatus());
                            } catch (SmppException e) {
                                log.error("Could not send response to client", e);
                            }
                        }

                        break;
                    }

                    case DataSM: {

                        long time = System.currentTimeMillis();
                        long message_id = time + al.incrementAndGet();

                        Message request = (Message) pdu;
                        String connection_name = request.getConnectionName();

                        int sequence_number = request.getSequenceNumber();

                        log.debug("Handle pdu with type '"+pdu.getType()+"', sequence_number '"+sequence_number+"', connection_name '"+connection_name+"', set id '"+message_id+"'.");

                        Provider provider = ConfigurationManager.getInstance().getProvider(connection_name);
                        log.debug("This connection name corresponds to the provider with name '"+provider.getName()+"'.");

                        Address source_address = request.getSourceAddress();
                        String service_number = source_address.getAddress();

                        Address smpp_destination_address = request.getDestinationAddress();
                        String destination_address_str = smpp_destination_address.getAddress();

                        String text = request.getMessage();
                        log.debug("Message: "+text);
                        log.debug("Id '"+message_id+"', service number '"+service_number+"', destination address '"+destination_address_str+"', text '"+text+"'.");

                        Delivery delivery = provider.getDelivery(service_number);
                        if (delivery != null){

                            int delivery_id = delivery.getId();
                            String login= delivery.getUser();

                            log.debug("Message id '"+message_id+"', service number '"+service_number+"', delivery id '"+delivery_id+"', user '"+login+"'.");

                            Sender sender = user_sender_table.get(login);
                            sender.addMessage(message_id, service_number,
                                    destination_address_str, text, sequence_number, connection_name, delivery_id);

                            if (request.getRegDeliveryReceipt() == RegDeliveryReceipt.None )
                                log.warn("Received DataSM with RegDeliveryReceipt.None .");

                        } else {
                            log.debug("Provider "+provider.getName()+" doesn't have delivery with service number '"+service_number+"'.");
                            SubmitSMResp submitSMResp = new SubmitSMResp();

                            submitSMResp.setStatus(mobi.eyeline.smpp.api.types.Status.SUBMITFAIL);
                            submitSMResp.setSequenceNumber(sequence_number);
                            submitSMResp.setConnectionName(connection_name);
                            submitSMResp.setTLV(new TLVString( (short)0x001D, "Provider "+provider.getName()+" doesn't have delivery with service number '"+service_number+"'.") );
                            try {
                                smppServer.send(submitSMResp, false);
                                log.debug("SUBMIT_SM_RESP sn: "+sequence_number+", con: "+connection_name+", status '"+submitSMResp.getStatus());
                            } catch (SmppException e) {
                                log.error("Could not send response to client", e);
                            }
                        }

                        break;
                    }

                    case DeliverSMResp:{

                        synchronized (read_write_monitor) {

                            DeliverSMResp resp = (DeliverSMResp) pdu;
                            int sequence_number = resp.getSequenceNumber();

                            log.debug("receive DeliverSMResp: sn=" + sequence_number);

                            Data data = sequence_number_receipt_table.get(sequence_number);
                            if (data != null) {
                                sequence_number_receipt_table.remove(sequence_number);
                                log.debug("Remove from memory deliver receipt data with sequence number " + sequence_number + " .");

                                data.setStatus(Status.DONE);
                                try {
                                    journal.write(data);
                                } catch (CouldNotWriteToJournalException e) {
                                    log.error(e);
                                }
                            } else {
                                log.warn("Couldn't find deliver receipt data with sequence number " + sequence_number);
                            }

                        }

                        break;
                    }

                }

                return true;
        }

    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Delivery

    private class DeliveryChangeListenerImpl implements DeliveryChangeListener {

        private SimpleDateFormat sdf = new SimpleDateFormat("yyMMddHHmmssSSS");

        @Override
        public void messageStateChanged(ChangeMessageStateEvent e) throws AdminException {
            log.debug(e);
            Properties p = e.getProperties();
            if (p!=null){
                if (p.containsKey("id")){
                    String s = p.getProperty("id");
                    long message_id = Long.parseLong(s);

                    mobi.eyeline.smpp.api.pdu.data.Address source_address, destination_address;
                    try {
                        source_address  = new mobi.eyeline.smpp.api.pdu.data.Address(p.getProperty("sa"));
                        destination_address = new mobi.eyeline.smpp.api.pdu.data.Address(e.getAddress().getAddress());
                    } catch (InvalidAddressFormatException e1) {
                        log.error(e);
                        throw new CouldNotReadMessageStateException("could.not.read.message.state", e1);
                    }

                    String connection_name = p.getProperty("con");

                    Date submit_date;
                    try {
                        submit_date = sdf.parse(p.getProperty("sd"));
                    } catch (ParseException e1) {
                        log.error(e1);
                        throw new CouldNotReadMessageStateException("could.not.read.message.state", e1);
                    }

                    Date done_date = e.getEventDate();

                    MessageState messageState = e.getMessageState();

                    int nsms = e.getNsms();

                    sendDeliveryReceipt(message_id, submit_date, done_date, connection_name, source_address, destination_address, nsms, messageState);
                } else {
                    log.warn("Couldn't find message identifier in the final log string.");
                }
            }
        }

        @Override
        public void deliveryStateChanged(ChangeDeliveryStatusEvent e) throws AdminException {
            log.debug(e);
        }
    }

    private void sendDeliveryReceipt(long message_id, Date submit_date, Date done_date, String connection_name,
                                           Address source_address, Address destination_address,
                                           int nsms, MessageState messageState) {

        synchronized (read_write_monitor) {

            FinalMessageState state;
            if (messageState == MessageState.Delivered) {
                state = FinalMessageState.DELIVRD;
            } else if (messageState == MessageState.Expired) {
                state = FinalMessageState.EXPIRED;
            } else {
                state = FinalMessageState.UNKNOWN;
            }

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

            long first_sending_time = System.currentTimeMillis();
            Data data = new Data();
            data.setMessageId(message_id);
            data.setConnectionName(connection_name);
            data.setSourceAddress(destination_address);
            data.setDestinationAddress(source_address);
            data.setFirstSendingTime(first_sending_time);
            data.setLastResendTime(first_sending_time);
            data.setDoneDate(done_date);
            data.setSubmitDate(submit_date);
            data.setFinalMessageState(state);
            data.setNsms(nsms);
            data.setSequenceNumber(sn);
            sequence_number_receipt_table.put(sn, data);
            log.debug("remember data: " + sn + " --> " + data +", table size: "+sequence_number_receipt_table.size());

            try {
                smppServer.send(deliverSM, false);
                log.debug("send DeliverSM: sn=" + sn + ", id=" + data.getMessageId());

                long send_receipt_time = System.currentTimeMillis();
                data.setLastResendTime(send_receipt_time);
                data.setStatus(Status.SEND);

                sequence_number_receipt_table.put(sn, data);
                try {
                    journal.write(data);
                } catch (CouldNotWriteToJournalException e) {
                    log.error(e);
                }
            } catch (SmppException e) {
                log.warn(e);

                long send_receipt_time = System.currentTimeMillis();
                data.setLastResendTime(send_receipt_time);
                data.setStatus(Status.NOT_SEND);

                sequence_number_receipt_table.put(sn, data);
                try {
                    journal.write(data);
                } catch (CouldNotWriteToJournalException e2) {
                    log.error(e2);
                }
            }
        }
    }

    private void resendDeliveryReceipts() {
        log.debug("Check receipts table to recent unanswered receipts ...");

        long current_time = System.currentTimeMillis();

        HashSet<Integer> timeout_expired_sequence_numbers = new HashSet<Integer>();
        HashSet<Integer> max_timeout_expired_sequence_numbers = new HashSet<Integer>();

        for (Map.Entry<Integer, Data> entry : sequence_number_receipt_table.entrySet()) {

            int sequence_number = entry.getKey();
            Data data = entry.getValue();
            long last_resend_time = data.getLastResendTime();
            long first_sending_time = data.getFirstSendingTime();


            if (current_time - first_sending_time < recend_receipts_max_timeout * 1000 * 60){

                if (current_time - last_resend_time >= recend_receipts_timeout * 1000)
                    timeout_expired_sequence_numbers.add(sequence_number);

            } else {
                    max_timeout_expired_sequence_numbers.add(sequence_number);
            }

        }

        for (Integer sn : max_timeout_expired_sequence_numbers) {
            synchronized (read_write_monitor) {
                Data data = sequence_number_receipt_table.remove(sn);
                log.debug("The maximum time of resending delivery receipt for message with id " + data.getMessageId() + " expired.");
                data.setStatus(Status.EXPIRED_MAX_TIMEOUT);
                try {
                    journal.write(data);
                } catch (CouldNotWriteToJournalException e) {
                    log.error(e);
                }
                log.debug("Remove deliver receipt data with sequence number " + sn + " and message id " +
                        data.getMessageId() + " from memory and write to journal with status " + Status.EXPIRED_MAX_TIMEOUT +
                        ", table size "+sequence_number_receipt_table.size()+".");
            }
        }

        for (Integer sn : timeout_expired_sequence_numbers) {
            if (!max_timeout_expired_sequence_numbers.contains(sn)) {
                synchronized (read_write_monitor) {
                    Data data = sequence_number_receipt_table.remove(sn);
                    log.warn("DeliverSM with sequence number " + sn + " expired. There was no DeliverSMResp within " + recend_receipts_timeout + " seconds. ");
                    data.setStatus(Status.EXPIRED_TIMEOUT);

                    try {
                        journal.write(data);
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
                        smppServer.send(deliverSM, false);
                        log.debug("resend DeliverSM: sn=" + new_sn + ", message_id=" + data.getMessageId());

                        long send_receipt_time = System.currentTimeMillis();
                        data.setLastResendTime(send_receipt_time);
                        data.setStatus(Status.SEND);

                        sequence_number_receipt_table.put(new_sn, data);
                        log.debug("remember data: " + new_sn + " --> " + data +", table size: "+sequence_number_receipt_table.size());


                        try {
                            journal.write(data);
                        } catch (CouldNotWriteToJournalException e) {
                            log.error(e);
                        }
                    } catch (ConnectionNotFoundException e) {
                        log.warn(e);

                        long send_receipt_time = System.currentTimeMillis();
                        data.setLastResendTime(send_receipt_time);
                        data.setStatus(Status.NOT_SEND);

                        sequence_number_receipt_table.put(new_sn, data);
                        try {
                            journal.write(data);
                        } catch (CouldNotWriteToJournalException e2) {
                            log.error(e2);
                        }
                    } catch (ConnectionNotEstablishedException e) {
                        log.warn(e);

                        long send_receipt_time = System.currentTimeMillis();
                        data.setLastResendTime(send_receipt_time);
                        data.setStatus(Status.NOT_SEND);

                        sequence_number_receipt_table.put(new_sn, data);
                        try {
                            journal.write(data);
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Update configuration

    class UpdateConfigServer extends Thread {

        private ServerSocket serverSocket;

        public UpdateConfigServer(String host, int port) throws IOException {
            if (host!= null && !host.isEmpty()){
                serverSocket = new ServerSocket();
                serverSocket.bind(new InetSocketAddress(host,port));
                log.debug("Set update server host to '"+host+"'.");
            } else{
                serverSocket = new ServerSocket(port);
            }
            this.start();
        }

        public void run() {
            while (!isInterrupted()) {
                try {
                    log.debug("Waiting for connections ...");
                    Socket client = serverSocket.accept();
                    log.debug("Accepted a connection from: " + client.getInetAddress());
                    new Connect(client);
                } catch (IOException e) {
                    log.error(e);
                }
            }
        }

    }

    class Connect extends Thread {

       private Socket client = null;
       private InputStream is = null;
       private OutputStream os = null;

       public Connect(Socket clientSocket) {
         client = clientSocket;
         try {
             is = client.getInputStream();
             os = client.getOutputStream();
         } catch(IOException e1) {
             try {
                client.close();
             }catch(Exception e) {
                log.error(e.getMessage());
             }
             return;
         }
         this.start();
       }


       public void run() {
           try {
               BufferReader buffer = new BufferReader(1024);
               buffer.fillFully(is, 4);
               int len = buffer.removeInt();

               if (log.isDebugEnabled()) log.debug("Received packet len=" + len);
               if (len > 0) buffer.fillFully(is, len);

               if (log.isDebugEnabled()) log.debug("PDU received: " + buffer.getHexDump());

               buffer.removeInt();

               int seqNum = buffer.removeInt();

               UpdateConfigResp configUpdateResp;

               try {
                   updateConfiguration();
                   configUpdateResp = new UpdateConfigResp(seqNum, 0, "ok");
               } catch (Exception e) {
                   configUpdateResp = new UpdateConfigResp(seqNum, 1, "Couldn't update configuration:"+e.getMessage());
               }

               serialize(configUpdateResp, os);

               is.close();
               os.close();
               client.close();
           } catch (IOException e) {
               log.error(e);
           }
       }

       private void serialize(mobi.eyeline.protogen.framework.PDU request, OutputStream os) throws IOException {
           BufferWriter writer = new BufferWriter();
           int pos = writer.size();
           writer.appendInt(0); // write 4 bytes for future length
           writer.appendInt(request.getTag());
           writer.appendInt(request.getSeqNum());
           request.encode(writer);
           int len = writer.size()-pos-4;
           writer.replaceInt( len,  pos); // fill first 4 bytes with actual length

           if (log.isDebugEnabled())
               log.debug("Sending PDU: " + writer.getHexDump());

           writer.writeBuffer(os);
           os.flush();
       }
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

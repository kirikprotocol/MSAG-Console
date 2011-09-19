package mobi.eyeline.dcpgw;

import mobi.eyeline.dcpgw.admin.UpdateConfigServer;
import mobi.eyeline.dcpgw.exeptions.CouldNotLoadJournalException;
import mobi.eyeline.dcpgw.exeptions.CouldNotWriteToJournalException;
import mobi.eyeline.dcpgw.exeptions.InitializationException;
import mobi.eyeline.dcpgw.journal.Data;
import mobi.eyeline.dcpgw.journal.Journal;
import mobi.eyeline.dcpgw.journal.Status;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.delivery.MessageState;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangesDetectorImpl;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.util.config.XmlConfigException;
import mobi.eyeline.smpp.api.ConnectionNotEstablishedException;
import mobi.eyeline.smpp.api.ConnectionNotFoundException;
import mobi.eyeline.smpp.api.PDUListener;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.pdu.*;
import mobi.eyeline.smpp.api.pdu.data.Address;
import mobi.eyeline.smpp.api.processing.ProcessingQueue;
import mobi.eyeline.smpp.api.processing.QueueException;
import mobi.eyeline.smpp.api.types.EsmMessageType;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

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

    private static SimpleDateFormat sdf = new SimpleDateFormat("yyMMddHHmm");
    private static SimpleDateFormat sdf2 = new SimpleDateFormat("ddHHmmss");
    private static Calendar cal = Calendar.getInstance();
    private static AtomicInteger ai = new AtomicInteger(0);

    private static int capacity;

    private static long sending_timeout;

    private static Hashtable<String, Sender> user_senders_map;

    // Подключение к информеру
    private static String informer_host;
    private static int informer_port;

    // Конфигурация шлюза
    private static ConfigurationManager cm;
    private static Properties config;
    private static Hashtable<String, String> user_password_table;
    private static Hashtable<String, Provider> connection_provider_table;

    public static void main(String args[]) {

        try {
            new Gateway();
        } catch (SmppException e) {
            log.error(e);
        } catch (InitializationException e) {
            log.error("Couldn't initialize gateway.", e);
        }

    }

    public Gateway() throws SmppException, InitializationException {
        log.debug("Try to initialize gateway ...");
        Runtime.getRuntime().addShutdownHook(this);

        // User's current working directory
        String user_dir = System.getProperty("user.dir");
        log.debug("User directory: " + user_dir);

        // Load configuration properties.
        String smpp_server_config_file = user_dir + File.separator + "conf" + File.separator + "config.properties";
        String smpp_endpoints_file = Utils.getProperty(config, "users.file", user_dir + File.separator + "conf" + File.separator + "endpoints.xml");
        String deliveries_file = Utils.getProperty(config, "deliveries.file", user_dir + File.separator + "conf" + File.separator + "deliveries.xml");
        cm = new ConfigurationManager(smpp_server_config_file, smpp_endpoints_file, deliveries_file);

        try {
            config = cm.loadSmppConfigurations();
            user_password_table = cm.loadUsers();
            connection_provider_table = cm.loadProviders();
        } catch (IOException e) {
            log.error(e);
            throw new InitializationException(e);
        } catch (XmlConfigException e) {
            log.error(e);
            throw new InitializationException(e);
        }

        // Инициализируем сервер обновляющий конфигурацию.
        String update_config_server_host = config.getProperty("update.config.server.host");

        String s = Utils.getProperty(config, "update.config.server.port");
        int update_config_server_port = Integer.parseInt(s);

        try {
            new UpdateConfigServer(update_config_server_host, update_config_server_port);
        } catch (IOException e) {
            log.error("Couldn't initialize update config server socket with host '" + update_config_server_host + "' and port '" + update_config_server_port + "'. ", e);
            throw new InitializationException(e);
        }

        // Инициализируем журнал.
        int max_journal_size_mb = Utils.getProperty(config, "max.journal.size.mb", 10);
        String journal_dir = Utils.getProperty(config, "journal.dir", System.getProperty("user.dir")+File.separator+"journal");
        journal = new Journal(journal_dir, max_journal_size_mb);

        try {
            sequence_number_receipt_table = journal.load();
        } catch (CouldNotLoadJournalException e) {
            log.error("Couldn't load journal.", e);
            throw new InitializationException(e);
        }

        // Инициализируем smpp сервер.

        PDUListenerImpl pduListener = new PDUListenerImpl();

        procQueue = new ProcessingQueue(config, pduListener, null);

        smppServer = new ConfigurableInRuntimeSmppServer(config, this);

        // Инициализируем детектор отслеживающий отчеты о доставке.
        String final_log_dir = Utils.getProperty(config, "final.log.dir", user_dir + File.separator + "final_log");

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

        // Инициализируем планировщик ответственный за передоставку отчетов о доставке.

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

        // Инициализируем планировщик ответственный за отчистку журнала
        ScheduledExecutorService clean_journal_scheduler = Executors.newSingleThreadScheduledExecutor();
        clean_journal_scheduler.scheduleWithFixedDelay(new Runnable() {

            @Override
            public void run() {
                synchronized (read_write_monitor) {
                    journal.clean();
                }
            }

        }, clean_journal_timeout, clean_journal_timeout, TimeUnit.MILLISECONDS);

        // Инициализируем параметры информера
        s = config.getProperty("informer.host");
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

        user_senders_map = new Hashtable<String, Sender>();

        log.debug("Successfully initialize gateway!");
    }

    public synchronized static void updateConfiguration() throws XmlConfigException, IOException, SmppException {
        log.debug("Try to update configuration ...");

        Hashtable<String, Provider> connection_provider_temp_table = cm.loadProviders();
        Hashtable<String, String> user_password_temp_table = cm.loadUsers();
        Properties new_config = cm.loadSmppConfigurations();

        smppServer.update(new_config);

        connection_provider_table = connection_provider_temp_table;
        user_password_table = user_password_temp_table;
        log.debug("Gateway configuration updated.");
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


    public static void sendSubmitSMResp(SubmitSMResp submitSMResp) throws SmppException {
        smppServer.send(submitSMResp, false);
    }

    public static void sendDeliveryReceipt(long message_id, Date submit_date, Date done_date, String connection_name,
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
            log.debug("remember data: " + sn + " --> " + data);

            try {
                smppServer.send(deliverSM, false);
                log.debug("resend DeliverSM: sn=" + sn + ", message_id=" + data.getMessageId());

                long send_receipt_time = System.currentTimeMillis();
                data.setLastResendTime(send_receipt_time);
                data.setStatus(Status.SEND);

                sequence_number_receipt_table.put(sn, data);
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

                sequence_number_receipt_table.put(sn, data);
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

                sequence_number_receipt_table.put(sn, data);
                try {
                    journal.write(data);
                } catch (CouldNotWriteToJournalException e2) {
                    log.error(e2);
                }
            } catch (SmppException e) {
                log.error(e);
            }
            log.debug("send DeliverSM: message_id=" + message_id + ", sn=" + sn + ".");
        }
    }

    public static void handleDeliverySMResp(PDU pdu) {
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
    }

    public static void resendDeliveryReceipts() {
        log.debug("Check receipts table to recent unanswered receipts ...");

        long current_time = System.currentTimeMillis();

        HashSet<Integer> timeout_expired_sequence_numbers = new HashSet<Integer>();
        HashSet<Integer> max_timeout_expired_sequence_numbers = new HashSet<Integer>();

        for (Map.Entry<Integer, Data> entry : sequence_number_receipt_table.entrySet()) {

            int sequence_number = entry.getKey();
            Data data = entry.getValue();
            long last_resend_time = data.getLastResendTime();
            long first_sending_time = data.getFirstSendingTime();

            if (current_time - first_sending_time < recend_receipts_max_timeout * 1000)

                if (current_time - last_resend_time >= recend_receipts_timeout * 1000)
                    timeout_expired_sequence_numbers.add(sequence_number);

                else

                    max_timeout_expired_sequence_numbers.add(sequence_number);

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
                log.debug("Remove deliver receipt data with sequence number " + sn + " and message id " + data.getMessageId() + " from memory and write to journal with status " + Status.EXPIRED_MAX_TIMEOUT + ".");
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

        log.debug("Successfully done recent task.");
    }

    public static Provider getProvider(String connection_name) {
        return connection_provider_table.get(connection_name);
    }

    synchronized public static Sender getSender(String user){

        Sender sender;
        if (user_senders_map.containsKey(user)){
            sender = user_senders_map.get(user);
        } else {
            log.debug("Try to initialize sender for user '"+user+"'.");
            sender = new Sender(informer_host, informer_port, user, user_password_table.get(user), capacity, sending_timeout, smppServer);
            user_senders_map.put(user, sender);
            sender.start();
        }

        return sender;
    }

}

package mobi.eyeline.dcpgw;

import mobi.eyeline.dcpgw.admin.UpdateConfigServer;
import mobi.eyeline.dcpgw.exeptions.CouldNotCleanJournalException;
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
import mobi.eyeline.informer.util.config.XmlConfig;
import mobi.eyeline.informer.util.config.XmlConfigException;
import mobi.eyeline.informer.util.config.XmlConfigParam;
import mobi.eyeline.informer.util.config.XmlConfigSection;
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
import java.io.FileInputStream;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

public class Gateway extends Thread implements PDUListener {

    private static Logger log = Logger.getLogger(Gateway.class);

    // Configuration file with deliveries data.
    private static File deliveries_file;

    // Configurations file with users data.
    private static File endpoints_file;

    // Table used to store user passwords.
    private static Hashtable<String, String> user_password_table;

    private static final Object read_write_monitor = new Object();

    private static ConfigurableInRuntimeSmppServer smppServer;

    private ProcessingQueue procQueue;

    protected DeliveryChangesDetectorImpl deliveryChangesDetector;

    protected FileSystem fileSystem;

    private static Journal journal;

    private static Hashtable<Integer, Data> sequence_number_receipt_table;

    private static int recend_receipts_timeout;
    private static int recend_receipts_max_timeout;

    public static final String CONNECTION_PREFIX = "smpp.sme.";

    private static Hashtable<String, Provider> connection_provider_table;

    private static SimpleDateFormat sdf = new SimpleDateFormat("yyMMddHHmm");
    private static SimpleDateFormat sdf2 = new SimpleDateFormat("ddHHmmss");
    private static Calendar cal = Calendar.getInstance();
    private static AtomicInteger ai = new AtomicInteger(0);

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
        String filename = user_dir + File.separator + "conf" + File.separator + "config.properties";
        Properties config = new Properties();
        try {
            config.load(new FileInputStream(filename));
        } catch (IOException e) {
            log.error("Couldn't load properties file '" + filename + "'.");
            throw new InitializationException(e);
        }

        // Load endpoints
        String endpoints_file_str = Utils.getProperty(config, "users.file", user_dir + File.separator + "conf" + File.separator + "endpoints.xml");
        endpoints_file = new File(endpoints_file_str);

        XmlConfig xmlConfig = new XmlConfig();

        try {
            xmlConfig.load(endpoints_file);
            XmlConfigSection endpoints_section = xmlConfig.getSection("endpoints");
            Collection<XmlConfigSection> c = endpoints_section.sections();
            for (XmlConfigSection s : c) {
                String endpoint_name = s.getName();

                XmlConfigParam p = s.getParam("enabled");
                if (p == null || p.getBool()) {
                    p = s.getParam("systemId");
                    String systemId = p.getString();

                    p = s.getParam("password");
                    String password = p.getString();

                    config.setProperty(CONNECTION_PREFIX + systemId + ".password", password);
                    log.debug("Load endpoint: name=" + endpoint_name + ", systemId=" + systemId + ", password=" + password + " .");
                } else {
                    log.debug("Endpoint '" + endpoint_name + "' is disabled.");
                }
            }
        } catch (XmlConfigException e) {
            throw new InitializationException(e);
        }

        connection_provider_table = new Hashtable<String, Provider>();

        user_password_table = new Hashtable<String, String>();

        String deliveries_file_str = Utils.getProperty(config, "deliveries.file", user_dir + File.separator + "conf" + File.separator + "deliveries.xml");
        deliveries_file = new File(deliveries_file_str);

        try {
            updateConfiguration();
        } catch (XmlConfigException e) {
            log.error("Couldn't update configuration.", e);
            throw new InitializationException(e);
        }

        // Initialize update configuration server.
        String update_config_server_host = config.getProperty("update.config.server.host");

        String s = Utils.getProperty(config, "update.config.server.port");
        int update_config_server_port = Integer.parseInt(s);

        try {
            new UpdateConfigServer(update_config_server_host, update_config_server_port);
        } catch (IOException e) {
            log.error("Couldn't initialize update config server socket with host '" + update_config_server_host + "' and port '" + update_config_server_port + "'. ", e);
            throw new InitializationException(e);
        }

        sequence_number_receipt_table = new Hashtable<Integer, Data>();
        journal = new Journal();


        try {
            journal.load();
        } catch (CouldNotLoadJournalException e) {
            log.error("Couldn't load journal.");
            throw new InitializationException(e);
        }

        // Initialize smpp server.

        PDUListenerImpl pduListener = new PDUListenerImpl();

        procQueue = new ProcessingQueue(config, pduListener, null);

        smppServer = new ConfigurableInRuntimeSmppServer(config, this);

        Manager.getInstance().setSmppServer(smppServer);

        // Initialize final log change detector.
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

        // Initialize the scheduler that will resend delivery receipts to smpp clients.

        int recend_receipts_interval = Utils.getProperty(config, "resend.receipts.interval.sec", 60);

        recend_receipts_timeout = Utils.getProperty(config, "resend.receipts.timeout.sec", 60);

        recend_receipts_max_timeout = Utils.getProperty(config, "resend.receipts.max.timeout.min", 720);

        ScheduledExecutorService scheduler1 = Executors.newSingleThreadScheduledExecutor();
        scheduler1.scheduleWithFixedDelay(new Runnable() {

            @Override
            public void run() {
                resendDeliveryReceipts();
            }

        }, recend_receipts_interval, recend_receipts_interval, TimeUnit.SECONDS);

        long clean_journal_timeout = Utils.getProperty(config, "clean.journal.timeout.msl", 60000);

        ScheduledExecutorService scheduler2 = Executors.newSingleThreadScheduledExecutor();
        scheduler2.scheduleWithFixedDelay(new Runnable() {

            @Override
            public void run() {
                synchronized (read_write_monitor) {
                    journal.clean();
                }
            }

        }, clean_journal_timeout, clean_journal_timeout, TimeUnit.MILLISECONDS);

        log.debug("Successfully initialize gateway!");
    }

    public synchronized static void updateConfiguration() throws XmlConfigException {
        log.debug("Try to update configuration ...");

        // Update deliveries file.
        XmlConfig xmlConfig = new XmlConfig();
        xmlConfig.load(deliveries_file);

        Hashtable<String, Provider> connection_provider_temp_table = new Hashtable<String, Provider>();

        XmlConfigSection providers_section = xmlConfig.getSection("providers");
        Collection<XmlConfigSection> providers_collection = providers_section.sections();

        for (XmlConfigSection s : providers_collection) {

            Provider provider = new Provider();

            String name = s.getName();
            provider.setName(name);

            XmlConfigParam p = s.getParam("endpoint_ids");
            String[] endpoint_ids = p.getStringArray(",");
            provider.setEndpointIds(endpoint_ids);

            p = s.getParam("description");
            String description = p.getString();
            provider.setDescription(description);

            XmlConfigSection deliveries_section = s.getSection("deliveries");
            Collection<XmlConfigSection> deliveries_collection = deliveries_section.sections();

            for (XmlConfigSection d_s : deliveries_collection) {

                Delivery delivery = new Delivery();

                int delivery_id = Integer.parseInt(d_s.getName());
                delivery.setId(delivery_id);

                p = d_s.getParam("user");
                String user = p.getString();
                delivery.setUser(user);

                p = d_s.getParam("services_numbers");
                String[] services_numbers = p.getStringArray(",");
                delivery.setServicesNumbers(services_numbers);

                provider.addDelivery(delivery);
                log.debug("Load " + delivery.toString());
            }

            for (String endpoint_id : endpoint_ids) connection_provider_temp_table.put(endpoint_id, provider);

            log.debug("Load " + provider.toString());
        }

        Hashtable<String, String> user_password_temp_table = new Hashtable<String, String>();

        XmlConfigSection users_section = xmlConfig.getSection("users");
        providers_collection = users_section.sections();
        for (XmlConfigSection s : providers_collection) {
            String login = s.getName();
            XmlConfigParam p = s.getParam("password");
            String password = p.getString();
            user_password_temp_table.put(login, password);
            log.debug("login: " + login + ", password: " + password);
        }

        connection_provider_table = connection_provider_temp_table;
        user_password_table = user_password_temp_table;

        // Update endpoint file.

        if (smppServer != null) {
            Hashtable<String, String> system_id_password_temp_table = loadEndpoints();
            smppServer.updateConnections(system_id_password_temp_table);
        }

        Manager.getInstance().setUserPasswordMap(user_password_table);

        log.debug("Successfully update configuration!");
    }

    public static Hashtable<String, String> loadEndpoints() throws XmlConfigException {
        log.debug("Try to load endpoints ...");
        Hashtable<String, String> result = new Hashtable<String, String>();

        XmlConfig xmlConfig = new XmlConfig();

        xmlConfig.load(endpoints_file);
        XmlConfigSection endpoints_section = xmlConfig.getSection("endpoints");
        Collection<XmlConfigSection> c = endpoints_section.sections();
        for (XmlConfigSection s : c) {
            String endpoint_name = s.getName();

            XmlConfigParam p = s.getParam("enabled");
            if (p == null || p.getBool()) {

                p = s.getParam("systemId");
                String systemId = p.getString();

                p = s.getParam("password");
                String password = p.getString();
                log.debug("Load endpoint: name=" + endpoint_name + ", systemId=" + systemId + ", password=" + password + " .");

                result.put(systemId, password);
            } else {
                log.debug("Endpoint with name '" + endpoint_name + "' is disabled.");
            }

        }

        log.debug("Successfully load endpoints!");
        return result;
    }


    public boolean handlePDU(PDU pdu) {
        try {
            procQueue.add(pdu, pdu.isMessage());
        } catch (QueueException e) {
            try {
                smppServer.send(((Request) pdu).getResponse(e.getStatus()));
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
        smppServer.send(submitSMResp);
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
                smppServer.send(deliverSM);
                log.debug("resend DeliverSM: sn="+sn+", message_id="+data.getMessageId());

                long send_receipt_time = System.currentTimeMillis();
                data.setLastResendTime(send_receipt_time);
                data.setStatus(Status.SEND);

                sequence_number_receipt_table.put(sn, data);
                try{
                    journal.write(data);
                } catch (CouldNotWriteToJournalException e){
                    log.error(e);
                }
            } catch(ConnectionNotFoundException e){
                log.warn(e);

                long send_receipt_time = System.currentTimeMillis();
                data.setLastResendTime(send_receipt_time);
                data.setStatus(Status.NOT_SEND);

                sequence_number_receipt_table.put(sn, data);
                try{
                    journal.write(data);
                } catch (CouldNotWriteToJournalException e2){
                    log.error(e2);
                }
            } catch (ConnectionNotEstablishedException e){
                log.warn(e);

                long send_receipt_time = System.currentTimeMillis();
                data.setLastResendTime(send_receipt_time);
                data.setStatus(Status.NOT_SEND);

                sequence_number_receipt_table.put(sn, data);
                try{
                    journal.write(data);
                } catch (CouldNotWriteToJournalException e2){
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

    public static void resendDeliveryReceipts(){
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

                if (current_time - last_resend_time >= recend_receipts_timeout * 1000) timeout_expired_sequence_numbers.add(sequence_number);

            else

                max_timeout_expired_sequence_numbers.add(sequence_number);

        }

        for(Integer sn: max_timeout_expired_sequence_numbers){
            synchronized (read_write_monitor) {
                Data data = sequence_number_receipt_table.remove(sn);
                log.debug("The maximum time of resending delivery receipt for message with id " +data.getMessageId() + " expired.");
                data.setStatus(Status.EXPIRED_MAX_TIMEOUT);
                try {
                    journal.write(data);
                } catch (CouldNotWriteToJournalException e) {
                    log.error(e);
                }
                log.debug("Remove deliver receipt data with sequence number "+sn+" and message id " +data.getMessageId() + " from memory and write to journal with status "+Status.EXPIRED_MAX_TIMEOUT+".");
            }
        }

        for(Integer sn: timeout_expired_sequence_numbers){
            if (!max_timeout_expired_sequence_numbers.contains(sn)){
                synchronized (read_write_monitor) {
                    Data data = sequence_number_receipt_table.remove(sn);
                    log.warn("DeliverSM with sequence number "+sn+" expired. There was no DeliverSMResp within "+recend_receipts_timeout+" seconds. ");
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
                        smppServer.send(deliverSM);
                        log.debug("resend DeliverSM: sn="+new_sn+", message_id="+data.getMessageId());

                        long send_receipt_time = System.currentTimeMillis();
                        data.setLastResendTime(send_receipt_time);
                        data.setStatus(Status.SEND);

                        sequence_number_receipt_table.put(new_sn, data);
                        try{
                            journal.write(data);
                        } catch (CouldNotWriteToJournalException e){
                            log.error(e);
                        }
                    } catch(ConnectionNotFoundException e){
                        log.warn(e);

                        long send_receipt_time = System.currentTimeMillis();
                        data.setLastResendTime(send_receipt_time);
                        data.setStatus(Status.NOT_SEND);

                        sequence_number_receipt_table.put(new_sn, data);
                        try{
                            journal.write(data);
                        } catch (CouldNotWriteToJournalException e2){
                            log.error(e2);
                        }
                    } catch (ConnectionNotEstablishedException e){
                        log.warn(e);

                        long send_receipt_time = System.currentTimeMillis();
                        data.setLastResendTime(send_receipt_time);
                        data.setStatus(Status.NOT_SEND);

                        sequence_number_receipt_table.put(new_sn, data);
                        try{
                            journal.write(data);
                        } catch (CouldNotWriteToJournalException e2){
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

    public static void addDeliveryReceiptData(int sequence_number, Data delivery_receipt_data) {
        sequence_number_receipt_table.put(sequence_number, delivery_receipt_data);
    }

    public static void removeDeliveryReceiptData(int sequence_number){
        sequence_number_receipt_table.remove(sequence_number);
    }

}

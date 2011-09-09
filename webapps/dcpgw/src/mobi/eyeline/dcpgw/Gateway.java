package mobi.eyeline.dcpgw;

import mobi.eyeline.dcpgw.admin.UpdateConfigServer;
import mobi.eyeline.dcpgw.exeptions.CouldNotCleanJournalException;
import mobi.eyeline.dcpgw.exeptions.CouldNotWriteToJournalException;
import mobi.eyeline.dcpgw.exeptions.InitializationException;
import mobi.eyeline.dcpgw.journal.Data;
import mobi.eyeline.dcpgw.journal.Status;
import mobi.eyeline.dcpgw.journal.Journal;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangesDetectorImpl;
import mobi.eyeline.informer.admin.filesystem.*;
import mobi.eyeline.informer.util.config.XmlConfig;
import mobi.eyeline.informer.util.config.XmlConfigException;
import mobi.eyeline.informer.util.config.XmlConfigParam;
import mobi.eyeline.informer.util.config.XmlConfigSection;
import mobi.eyeline.smpp.api.*;
import mobi.eyeline.smpp.api.pdu.*;
import mobi.eyeline.smpp.api.processing.ProcessingQueue;
import mobi.eyeline.smpp.api.processing.QueueException;
import org.apache.log4j.Logger;

import java.io.*;
import java.util.*;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

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

    private static Hashtable<Integer, Data> recepts_table;

    private static int recend_receipts_timeout;
    private static int recend_receipts_max_timeout;

    public static final String CONNECTION_PREFIX = "smpp.sme.";

    private static Hashtable<String, Provider> connection_provider_table;

    public Gateway() throws SmppException, InitializationException{
        log.debug("Try to initialize gateway ...");
        Runtime.getRuntime().addShutdownHook(this);

        // User's current working directory
        String user_dir = System.getProperty("user.dir");
        log.debug("User directory: "+user_dir);

        // Load configuration properties.
        String filename = user_dir+File.separator+"conf"+File.separator+"config.properties";
        Properties config = new Properties();
        try {
            config.load(new FileInputStream(filename));
        } catch (IOException e) {
            log.error("Couldn't load properties file '"+filename+"'.");
            throw new InitializationException(e);
        }

        // Load endpoints
        String endpoints_file_str = Utils.getProperty(config, "users.file", user_dir+File.separator+"conf"+File.separator+"endpoints.xml" );
        endpoints_file = new File(endpoints_file_str);

        Hashtable<String, String> systemId_password_table;
        try {
            systemId_password_table = loadEndpoints();
        } catch (XmlConfigException e) {
            log.error(e);
            throw new InitializationException(e);
        }

        for (Map.Entry<String, String> entry : systemId_password_table.entrySet()) {
            String systemId = entry.getKey();
            String password = entry.getValue();
            String connection_property_name = CONNECTION_PREFIX+systemId+".password";
            config.setProperty(connection_property_name, password);
        }

        connection_provider_table = new Hashtable<String, Provider>();

        user_password_table = new Hashtable<String, String>();

        String deliveries_file_str = Utils.getProperty(config, "deliveries.file", user_dir+File.separator+"conf"+File.separator+"deliveries.xml" );
        deliveries_file = new File(deliveries_file_str);

        PDUListenerImpl pduListener = new PDUListenerImpl();

        try {
            updateConfiguration();
        } catch (XmlConfigException e) {
            log.error("Couldn't update configuration.", e);
            throw new InitializationException(e);
        }

        recepts_table = new Hashtable<Integer, Data>();

        // Initialize update configuration server.
        String update_config_server_host = config.getProperty("update.config.server.host");

        String s = Utils.getProperty(config,"update.config.server.port");
        int update_config_server_port = Integer.parseInt(s);

        try {
            new UpdateConfigServer(update_config_server_host, update_config_server_port);
        } catch (IOException e) {
            log.error("Couldn't initialize update config server socket with host '"+update_config_server_host+"' and port '"+update_config_server_port+"'. ", e);
            throw new InitializationException(e);
        }

        journal = new Journal();

        // Initialize smpp server.

        procQueue = new ProcessingQueue(config, pduListener, null);

        smppServer = new ConfigurableInRuntimeSmppServer(config, this);

        Manager.getInstance().setSmppServer(smppServer);

        // Initialize final log change detector.
        String final_log_dir = Utils.getProperty(config, "final.log.dir", user_dir+File.separator+"final_log");

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
                try {
                    resendDeliveryReceipts();
                } catch (SmppException e) {
                    log.error(e);
                    // todo ?
                } catch (CouldNotWriteToJournalException e) {
                    log.error(e);
                    // todo ?
                }
            }

        }, recend_receipts_interval, recend_receipts_interval, TimeUnit.SECONDS);

        long clean_journal_timeout = Utils.getProperty(config, "clean.journal.timeout.msl", 60000);

        ScheduledExecutorService scheduler2 = Executors.newSingleThreadScheduledExecutor();
        scheduler2.scheduleWithFixedDelay(new Runnable() {

            @Override
            public void run() {
                try {
                    cleanJournal();
                } catch (CouldNotCleanJournalException e) {
                    log.error(e);
                    // todo ?
                }
            }

        }, clean_journal_timeout, clean_journal_timeout, TimeUnit.MILLISECONDS);

        log.debug("Successfully initialize gateway!");
    }

    public static Hashtable<String, String> loadEndpoints() throws XmlConfigException {
        log.debug("Try to load endpoints ...");
        Hashtable<String, String> result = new Hashtable<String, String>();

        XmlConfig xmlConfig = new XmlConfig();

        xmlConfig.load(endpoints_file);
        XmlConfigSection endpoints_section = xmlConfig.getSection("endpoints");
        Collection<XmlConfigSection> c = endpoints_section.sections();
        for(XmlConfigSection s: c){
            String endpoint_name = s.getName();

            XmlConfigParam p = s.getParam("enabled");
            boolean enabled = p.getBool();

            if (enabled){

                p = s.getParam("systemId");
                String systemId = p.getString();

                p = s.getParam("password");
                String password = p.getString();
                log.debug("Load endpoint: name="+endpoint_name+", systemId="+systemId+", password="+password+" .");

                result.put(systemId, password);
            } else {
                log.debug("Endpoint with name '"+endpoint_name+"' is disabled.");
            }

        }

        log.debug("Successfully load endpoints!");
        return result;
    }


    public static void updateConfiguration() throws XmlConfigException {
        log.debug("Try to update configuration ...");

        // Update deliveries file.
        XmlConfig xmlConfig = new XmlConfig();
        xmlConfig.load(deliveries_file);

        Hashtable<String, Provider> connection_provider_temp_table = new Hashtable<String, Provider>();

        XmlConfigSection providers_section = xmlConfig.getSection("providers");
        Collection<XmlConfigSection> providers_collection = providers_section.sections();

        for(XmlConfigSection s: providers_collection){

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

            for(XmlConfigSection d_s: deliveries_collection){

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
                log.debug("Load "+delivery.toString());
            }

            for(String endpoint_id: endpoint_ids) connection_provider_temp_table.put(endpoint_id, provider);

            log.debug("Load "+provider.toString());
        }

        Hashtable<String, String> user_password_temp_table = new Hashtable<String, String>();

        XmlConfigSection users_section = xmlConfig.getSection("users");
        providers_collection = users_section.sections();
        for(XmlConfigSection s: providers_collection){
            String login = s.getName();
            XmlConfigParam p = s.getParam("password");
            String password = p.getString();
            user_password_temp_table.put(login, password);
            log.debug("login: "+login+", password: "+password);
        }

        connection_provider_table = connection_provider_temp_table;
        user_password_table = user_password_temp_table;

        // Update endpoint file.

        if (smppServer != null){
            Hashtable<String, String> system_id_password_temp_table = loadEndpoints();
            smppServer.updateConnections(system_id_password_temp_table);
        }

        Manager.getInstance().setUserPasswordMap(user_password_table);

        log.debug("Successfully update configuration!");
    }

    public boolean handlePDU(PDU pdu) {
        try {
            procQueue.add(pdu, pdu.isMessage());
        } catch (QueueException e) {
            try {
                smppServer.send(((Request)pdu).getResponse(e.getStatus()));
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

    public static void main(String args[]){

        try {
            new Gateway();
        } catch (SmppException e) {
            log.error(e);
        } catch (InitializationException e) {
            log.error("Couldn't initialize gateway.", e);
        }

    }

    public static void sendSubmitSMResp(SubmitSMResp submitSMResp) throws SmppException {
        smppServer.send(submitSMResp);
    }

    public static void sendDeliveryReceipt(long message_id, Message delivery_receipt) throws SmppException, CouldNotWriteToJournalException{
        synchronized (read_write_monitor){

            int sequence_number = delivery_receipt.getSequenceNumber();
            log.debug("Try to send delivery receipt with message id '"+message_id+"' and sequence number '"+sequence_number+"'.");
            smppServer.send(delivery_receipt);
            log.debug("Successfully send delivery receipt with sequence number '"+sequence_number+"'.");

            long first_sending_time = System.currentTimeMillis();

            Data data = new Data(message_id, first_sending_time, first_sending_time );
            journal.write(sequence_number, data, Status.SEND);

            recepts_table.put(sequence_number, data);
            log.debug("Remember to the memory: " + sequence_number+" --> " + data);

        }
    }

    public static void handleDeliverySMResp(PDU pdu){
        synchronized (read_write_monitor){

            DeliverSMResp resp = (DeliverSMResp) pdu;
            int sequence_number = resp.getSequenceNumber();
            log.debug("DeliverSMResp: sequence_number="+sequence_number);

            Data data = recepts_table.get(sequence_number);

            if (data != null){
                try {
                    journal.write(sequence_number, data, Status.DONE);
                } catch (CouldNotWriteToJournalException e) {
                    log.error(e);
                    // todo ?
                }

                recepts_table.remove(sequence_number);

                log.debug("Remove from memory: " + sequence_number);

            } else {
                log.warn("Couldn't find deliver receipt with sequence number "+sequence_number);
            }

        }
    }

    public static void cleanJournal() throws CouldNotCleanJournalException {
        synchronized (read_write_monitor){
            journal.cleanJournal();
        }
    }

    public static void resendDeliveryReceipts() throws SmppException, CouldNotWriteToJournalException {
        log.debug("Check receipts table to recent unanswered receipts ...");

        long current_time = System.currentTimeMillis();

        for (Map.Entry<Integer, Data> entry : recepts_table.entrySet()) {

            int sequence_number = entry.getKey();
            Data data = entry.getValue();
            long last_resend_time = data.getLastResendTime();
            long first_sending_time = data.getFirstSendingTime();
            long message_id = data.getMessageId();
            //DateFormat df = DateFormat.getDateTimeInstance();

            if (current_time - first_sending_time < recend_receipts_max_timeout*1000){

                if (current_time - last_resend_time > recend_receipts_timeout*1000){

                    synchronized (read_write_monitor){
                        log.debug("sequence_number: "+sequence_number+", first sending time: "+first_sending_time+", last resending time: "+last_resend_time);
                        Message rcpt = Manager.getInstance().getReceiptMessage(message_id);

                        smppServer.send(rcpt);

                        long send_receipt_time = System.currentTimeMillis();
                        data.setLastResendTime(send_receipt_time);

                        journal.write(sequence_number, data, Status.SEND);
                    }

                }

            } else {

                synchronized (read_write_monitor){
                    log.debug("The maximum time of resending delivery receipt with sequence number " + sequence_number + " expired.");
                    recepts_table.remove(sequence_number);
                    journal.write(sequence_number, data, Status.DONE);
                    log.debug("Remove deliver receipt data with sequence number "+sequence_number+" from memory and write to journal with status DONE.");
                }

            }

        }

        log.debug("Successfully done recent task.");
    }

    public static Provider getProvider(String connection_name){
        return connection_provider_table.get(connection_name);
    }

}

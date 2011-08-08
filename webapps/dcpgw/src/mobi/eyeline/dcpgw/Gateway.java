package mobi.eyeline.dcpgw;

import mobi.eyeline.dcpgw.admin.UpdateConfigServer;
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
import mobi.eyeline.smpp.api.pdu.*;
import mobi.eyeline.smpp.api.processing.ProcessingQueue;
import mobi.eyeline.smpp.api.processing.QueueException;
import org.apache.log4j.Logger;

import java.io.*;
import java.util.*;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import mobi.eyeline.smpp.api.SmppServer;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.PDUListener;

public class Gateway extends Thread implements PDUListener {

    private static Logger log = Logger.getLogger(Gateway.class);

    // Configuration file with deliveries data.
    private static File deliveries_file;

    // Configurations file with users data.
    private static File endpoints_file;

    // Table used to store user passwords.
    private static Hashtable<String, String> user_password_table;

    // Table used to map delivery identifier and user..
    private static Hashtable<Integer, String> delivery_id_user_table;

    // Table used to map service number and delivery identifier.
    private static Hashtable<String, Integer> service_number_delivery_id_table;

    //
    private static Hashtable<String, String[]> provider_systemIds_table;

    private static final Object read_write_monitor = new Object();

    private static SmppServer smppServer;

    private ProcessingQueue procQueue;

    protected DeliveryChangesDetectorImpl deliveryChangesDetector;

    protected FileSystem fileSystem;

    private static Journal journal;

    private static Hashtable<Integer, Data> recepts_table;

    private static int recend_receipts_timeout;
    private static int recend_receipts_max_timeout;

    private static PDUListenerImpl pduListener;

    public static final String CONNECTION_PREFIX = "smpp.sme.";

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

        String endpoints_file_str = Utils.getProperty(config, "users.file", user_dir+File.separator+"conf"+File.separator+"endpoints.xml" );
        endpoints_file = new File(endpoints_file_str);

        XmlConfig xmlConfig = new XmlConfig();
        try {
            xmlConfig.load(endpoints_file);
            XmlConfigSection endpoints_section = xmlConfig.getSection("endpoints");
            Collection<XmlConfigSection> c = endpoints_section.sections();
            for(XmlConfigSection s: c){
                String endpoint_name = s.getName();

                XmlConfigParam p = s.getParam("systemId");
                String systemId = p.getString();

                p = s.getParam("password");
                String password = p.getString();
                log.debug("Load: endpoint name="+endpoint_name+", systemId="+systemId+", password="+password);

                String connection_property_name = CONNECTION_PREFIX+systemId+".password";

                config.setProperty(connection_property_name, password);
                log.debug("Set "+connection_property_name+" --> password");
            }
        } catch (XmlConfigException e) {
            log.error(e);
            throw new InitializationException(e);
        }

        // Load users and deliveries configuration.
        user_password_table = new Hashtable<String, String>();

        String deliveries_file_str = Utils.getProperty(config, "deliveries.file", user_dir+File.separator+"conf"+File.separator+"deliveries.xml" );
        deliveries_file = new File(deliveries_file_str);
        delivery_id_user_table = new Hashtable<Integer, String>();
        service_number_delivery_id_table = new Hashtable<String, Integer>();

        pduListener = new PDUListenerImpl();

        provider_systemIds_table = new Hashtable<String, String[]>();

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

        smppServer = new SmppServer(config, this);

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

        ScheduledExecutorService scheduler = Executors.newSingleThreadScheduledExecutor();
        scheduler.scheduleWithFixedDelay(new Runnable() {

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

        log.debug("Successfully initialize gateway!");
    }

    public static void updateConfiguration() throws XmlConfigException {
        log.debug("Try to load update configuration ...");

        XmlConfig xmlConfig = new XmlConfig();
        xmlConfig.load(deliveries_file);

        Hashtable<String, String[]> t = new Hashtable<String, String[]>();
        Hashtable<Integer, String> t1 = new Hashtable<Integer, String>();
        Hashtable<String, Integer> t2 = new Hashtable<String, Integer>();

        XmlConfigSection providers_section = xmlConfig.getSection("providers");
        Collection<XmlConfigSection> c = providers_section.sections();

        for(XmlConfigSection s: c){
            String provider_name = s.getName();
            XmlConfigParam p = s.getParam("endpoint_ids");

            String[] ar = p.getStringArray(",");
            t.put(provider_name, ar);
            log.debug("put "+provider_name+" --> "+p.getString());

            XmlConfigSection deliveries_section = s.getSection("deliveries");
            Collection<XmlConfigSection> c2 = deliveries_section.sections();
            for(XmlConfigSection d_s: c2){
                int delivery_id = Integer.parseInt(d_s.getName());
                p = d_s.getParam("user");
                String user = p.getString();

                t1.put(delivery_id, user);
                log.debug("delivery_id: "+delivery_id+", user:"+user);

                XmlConfigParam p2 = d_s.getParam("services_numbers");
                ar = p2.getStringArray(",");

                for(String a: ar){
                    String service_number = a.trim();
                    t2.put(service_number, delivery_id);
                    log.debug("service_number: "+service_number+", delivery_id: "+delivery_id);
                }
            }

        }

        Hashtable<String, String> t3 = new Hashtable<String, String>();

        XmlConfigSection users_section = xmlConfig.getSection("users");
        c = users_section.sections();
        for(XmlConfigSection s: c){
            String login = s.getName();
            XmlConfigParam p = s.getParam("password");
            String password = p.getString();
            t3.put(login, password);
            log.debug("login: "+login+", password: "+password);
        }



        provider_systemIds_table = t;
        delivery_id_user_table = t1;
        service_number_delivery_id_table = t2;

        user_password_table = t3;

        Manager.getInstance().setUserPasswordMap(user_password_table);

        pduListener.update(delivery_id_user_table, service_number_delivery_id_table);

        log.debug("Successfully update deliveries ...");
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

}

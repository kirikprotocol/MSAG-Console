package mobi.eyeline.dcpgw;

import mobi.eyeline.dcpgw.exeptions.CouldNotWriteToJournalException;
import mobi.eyeline.dcpgw.exeptions.InitializationException;
import mobi.eyeline.dcpgw.exeptions.UpdateConfigurationException;
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
import mobi.eyeline.smpp.api.types.EsmMessageType;
import mobi.eyeline.smpp.api.types.RegDeliveryReceipt;
import org.apache.log4j.Logger;

import java.io.*;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

import mobi.eyeline.smpp.api.SmppServer;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.PDUListener;
import mobi.eyeline.smpp.api.pdu.data.Address;

public class Gateway extends Thread implements PDUListener {

    private static final Object read_write_monitor = new Object();

    private static Logger log = Logger.getLogger(Gateway.class);

    private static SmppServer smppServer;

    private ProcessingQueue procQueue;

    private Hashtable<String, String> user_password_map;
    private Hashtable<Integer, String> delivery_id_user_map;
    private Hashtable<String, Integer> service_number_delivery_id_map;

    private String mapping_filename;

    private AtomicLong al = new AtomicLong(0);

    private AtomicInteger ai = new AtomicInteger(0);

    private SimpleDateFormat sdf = new SimpleDateFormat("ddHHmmss");

    protected DeliveryChangesDetectorImpl deliveryChangesDetector;

    protected FileSystem fileSystem;

    private static final Journal journal = new Journal();

    private Calendar cal;

    private static Hashtable<Integer, Data> recepts_table;

    private static int recend_receipts_interval, recend_receipts_timeout, recend_receipts_max_timeout;

    public Gateway() throws SmppException, InitializationException{

        Runtime.getRuntime().addShutdownHook(this);

        Properties config = new Properties();

        String userDir = System.getProperty("user.dir");
        String filename = userDir+File.separator+"conf"+File.separator+"dcpgw.properties";

        mapping_filename = userDir+File.separator+"conf"+File.separator+"config.xml";

        try {
            config.load(new FileInputStream(filename));
        } catch (IOException e) {
            log.debug("Couldn't load properties file.");
            throw new InitializationException(e);
        }

        user_password_map = new Hashtable<String, String>();
        delivery_id_user_map = new Hashtable<Integer, String>();
        service_number_delivery_id_map = new Hashtable<String, Integer>();

        recepts_table = new Hashtable<Integer, Data>();

        try {
            updateDeliveries();
        } catch (UpdateConfigurationException e) {
            log.error(e);
            throw new InitializationException(e);
        }

        cal = Calendar.getInstance();

        procQueue = new ProcessingQueue(config,

            new PDUListener() {

                public boolean handlePDU(PDU pdu) {

                    switch (pdu.getType()) {

                        case SubmitSM:{

                            long time = System.currentTimeMillis();
                            long message_id = time + al.incrementAndGet();

                            Message request = (Message) pdu;
                            String connection_name = request.getConnectionName();
                            int sequence_number = request.getSequenceNumber();

                            log.debug("Handle pdu with type '"+pdu.getType()+"', sequence_number '"+sequence_number+"', set id '"+message_id+"'.");

                            Address source_address = request.getSourceAddress();
                            String source_address_str = source_address.getAddress();

                            Address smpp_destination_address = request.getDestinationAddress();
                            String destination_address_str = smpp_destination_address.getAddress();

                            String text = request.getMessage();
                            log.debug("Id '"+message_id+"', source address '"+source_address_str+"', destination address '"+destination_address_str+"', text '"+text+"'.");

                            int delivery_id = service_number_delivery_id_map.get(source_address_str);

                            String login = delivery_id_user_map.get(delivery_id);
                            log.debug("Id '"+message_id+"', service_number '"+source_address_str+"', delivery_id '"+delivery_id+"', user '"+login+"'.");

                            Manager.getInstance().getSender(login).addMessage(message_id, destination_address_str, text, sequence_number, connection_name, delivery_id);

                            if( request.getRegDeliveryReceipt() != RegDeliveryReceipt.None ) {

                                Message rcpt = request.getAnswer();
                                rcpt.setEsmMessageType(EsmMessageType.DeliveryReceipt);

                                Date date = cal.getTime();
                                int sn = Integer.parseInt(sdf.format(date))+ai.incrementAndGet();
                                rcpt.setSequenceNumber(sn);

                                Manager.getInstance().rememberReceiptMessage(message_id, rcpt);
                            }

                            break;
                        }
                        case DataSM: {



                            break;
                        }

                        case DeliverSMResp:{
                            synchronized (read_write_monitor){

                                log.debug("Handle pdu with type "+pdu.getType());

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

                            break;
                        }

                    }

                    return true;
                }
            }

        , null);

        smppServer = new SmppServer(config, this);

        Manager.getInstance().setSmppServer(smppServer);

        fileSystem = FileSystem.getFSForSingleInst();
        try {
            deliveryChangesDetector = new DeliveryChangesDetectorImpl(new File(userDir+File.separator+"final_log"), fileSystem);

            DeliveryChangeListenerImpl deliveryChangeListener = new DeliveryChangeListenerImpl();
            deliveryChangesDetector.addListener(deliveryChangeListener);

            deliveryChangesDetector.start();
        } catch (InitException e) {
            log.error(e);
            System.exit(0);
        }


        // Initialize the scheduler that will resend delivery receipts to smpp clients.

        String s = config.getProperty("resend.receipts.interval.sec");
        if (s != null && !s.isEmpty()){
            recend_receipts_interval = Integer.parseInt(s);
            log.debug("Initialization parameter: resend_receipts_timeout="+recend_receipts_interval);
        } else {
            log.warn("Parameter 'resend.receipts.interval.sec' is not found or is empty, set default value to " + recend_receipts_interval + " seconds.");
        }

        s = config.getProperty("resend.receipts.timeout.sec");

        if (s != null && !s.isEmpty()){
            recend_receipts_timeout = Integer.parseInt(s);
            log.debug("Initialization parameter: resend_receipts_timeout="+recend_receipts_timeout);
        } else {
            log.warn("Parameter 'resend.receipts.timeout.sec' is not found or is empty, set default value to " + recend_receipts_timeout + " seconds.");
        }

        s = config.getProperty("resend.receipts.max.timeout.min");

        if (s != null && !s.isEmpty()){
            recend_receipts_max_timeout = Integer.parseInt(s);
            log.debug("Initialization parameter: resend_receipts_timeout="+recend_receipts_max_timeout);
        } else {
            log.warn("Parameter 'resend.receipts.max.timeout.min' is not found or is empty, set default value to " + recend_receipts_max_timeout + " minutes.");
        }


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

        }, recend_receipts_timeout, recend_receipts_timeout, TimeUnit.SECONDS);

    }

    private void updateDeliveries() throws UpdateConfigurationException{
        log.debug("Try to update deliveries ...");
        try {

            XmlConfig xmlConfig = new XmlConfig();
            xmlConfig.load(new File(mapping_filename));
            XmlConfigSection users_section = xmlConfig.getSection("users");
            Collection<XmlConfigSection> c = users_section.sections();
            for(XmlConfigSection s: c){
                String login = s.getName();

                XmlConfigParam p = s.getParam("password");
                String password = p.getString();
                log.debug("login: "+login+", password: "+password);
                user_password_map.put(login, password);
            }

            XmlConfigSection deliveries_section = xmlConfig.getSection("deliveries");
            c = deliveries_section.sections();
            for(XmlConfigSection s: c){

                int delivery_id = Integer.parseInt(s.getName());

                XmlConfigParam p = s.getParam("user");
                String user = p.getString();
                delivery_id_user_map.put(delivery_id, user);
                log.debug("delivery_id: "+delivery_id+", user:"+user);

                XmlConfigParam p2 = s.getParam("services_numbers");
                String[] ar = p2.getStringArray(",");

                for(String a: ar){
                    String service_number = a.trim();
                    service_number_delivery_id_map.put(service_number, delivery_id);
                    log.debug("service_number: "+service_number+", delivery_id: "+delivery_id);
                }
            }

            Manager.getInstance().setUserPasswordMap(user_password_map);

        } catch (XmlConfigException e) {
            log.debug(e);
            // todo ?
        }
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

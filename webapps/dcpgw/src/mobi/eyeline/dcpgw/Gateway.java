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
import mobi.eyeline.smpp.api.pdu.*;
import mobi.eyeline.smpp.api.processing.ProcessingQueue;
import mobi.eyeline.smpp.api.processing.QueueException;
import mobi.eyeline.smpp.api.types.EsmMessageType;
import mobi.eyeline.smpp.api.types.RegDeliveryReceipt;
import org.apache.log4j.Logger;

import java.io.*;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

import mobi.eyeline.smpp.api.SmppServer;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.PDUListener;
import mobi.eyeline.smpp.api.pdu.data.Address;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;
import org.jdom.transform.JDOMSource;
import org.xml.sax.SAXException;

import javax.xml.transform.stream.StreamSource;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;
import javax.xml.validation.Validator;


public class Gateway extends Thread implements PDUListener {

    private static final Object read_write_monitor = new Object();

    private static Logger log = Logger.getLogger(Gateway.class);

    private static SmppServer smppServer;

    private ProcessingQueue procQueue;

    private Hashtable<String, String> user_password_map;
    private Hashtable<Integer, String> delivery_id_user_map;
    private Hashtable<Long, Integer> service_number_delivery_id_map;

    private String mapping_filename;

    private Schema schema;

    private AtomicLong al = new AtomicLong(0);

    private AtomicInteger ai = new AtomicInteger(0);

    private SimpleDateFormat sdf = new SimpleDateFormat("ddHHmmss");

    protected DeliveryChangesDetectorImpl deliveryChangesDetector;

    protected FileSystem fileSystem;

    private static final Journal journal = new Journal();

    private Calendar cal;

    private static Hashtable<Integer, Data> table;

    public Gateway() throws SmppException, InitializationException{

        Runtime.getRuntime().addShutdownHook(this);

        Properties config = new Properties();

        String userDir = System.getProperty("user.dir");
        String filename = userDir+File.separator+"config"+File.separator+"dcpgw.properties";

        mapping_filename = userDir+File.separator+"config"+File.separator+"mapping.xml";
        String schema_filename = userDir+File.separator+"config"+File.separator+"mapping.xsd";

        try {
            config.load(new FileInputStream(filename));
        } catch (IOException e) {
            log.debug("Couldn't load properties file.");
            throw new InitializationException(e);
        }

        user_password_map = new Hashtable<String, String>();
        delivery_id_user_map = new Hashtable<Integer, String>();
        service_number_delivery_id_map = new Hashtable<Long, Integer>();

        table = new Hashtable<Integer, Data>();

        System.setProperty("javax.xml.validation.SchemaFactory:http://www.w3.org/XML/XMLSchema/v1.1",
                           "org.apache.xerces.jaxp.validation.XMLSchema11Factory");
        SchemaFactory sf = SchemaFactory.newInstance("http://www.w3.org/XML/XMLSchema/v1.1");
        try {
            InputStream inputStream = new FileInputStream(schema_filename);
            StreamSource ss = new StreamSource(inputStream);
            schema = sf.newSchema(ss);
        } catch (FileNotFoundException e) {
            log.error(e);
            throw new InitializationException(e);
        } catch (SAXException e) {
            log.error(e);
            throw new InitializationException(e);
        }

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

                            long service_number = Long.parseLong(source_address_str);

                            int delivery_id = service_number_delivery_id_map.get(service_number);

                            String login = delivery_id_user_map.get(delivery_id);
                            log.debug("Id '"+message_id+"', service_number '"+service_number+"', delivery_id '"+delivery_id+"', user '"+login+"'.");

                            Manager.getInstance().getSender(login).addMessage(message_id, destination_address_str, text, sequence_number, connection_name, delivery_id);

                            if( request.getRegDeliveryReceipt() != RegDeliveryReceipt.None ) {
                                Message rcpt = request.getAnswer();
                                rcpt.setEsmMessageType(EsmMessageType.DeliveryReceipt);

                                Date date = cal.getTime();
                                int sn = Integer.parseInt(sdf.format(date))+ai.incrementAndGet();
                                rcpt.setSequenceNumber(sn);

                                log.debug("seq num:" + rcpt.getSequenceNum());

                                Manager.getInstance().rememberReceiptMessage(message_id, rcpt);
                            }

                            break;
                        }
                        case DataSM: {



                            break;
                        }

                        case DeliverSMResp:{
                            log.debug("Handle pdu with type "+pdu.getType());

                            DeliverSMResp resp = (DeliverSMResp) pdu;
                            int sequence_number = resp.getSequenceNumber();
                            log.debug("DeliverSMResp: sequence_number="+sequence_number);

                            Data data = table.get(sequence_number);

                            if (data != null){

                                synchronized (read_write_monitor){

                                    try {
                                        journal.write(sequence_number, data, Status.DONE);
                                    } catch (CouldNotWriteToJournalException e) {
                                        log.error(e);
                                        // todo ?
                                    }

                                    table.remove(sequence_number);
                                    log.debug("Remove from memory: " + sequence_number);

                                }

                            } else {
                                log.warn("Couldn't find deliver receipt with sequence number "+sequence_number);
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

        }

    }

    private void validateXMLSchema11(Document document) throws SAXException, IOException {
        log.debug("Try to validate in-memory document ...");
        Validator v = schema.newValidator();
        JDOMSource in = new JDOMSource(document);
        v.validate(in);
        log.debug("Successfully validate xml file!");
    }

    private void updateDeliveries() throws UpdateConfigurationException{
        log.debug("Try to update deliveries ...");
        try {

            SAXBuilder builder = new SAXBuilder();
            Document document = builder.build(new File(mapping_filename));
            validateXMLSchema11(document);

            Element deliveries_el = document.getRootElement();
            Element users_el = deliveries_el.getChild("Users");
            if (users_el != null){
                List users_list = users_el.getChildren("User");
                for(int i=0; i<users_list.size(); i++){

                    Element user_el = (Element) users_list.get(i);
                    String login = user_el.getAttributeValue("login");
                    String password = user_el.getAttributeValue("password");
                    log.debug("User "+(i+1)+": login="+login+" password="+password);
                    user_password_map.put(login, password);
                }
            } else {
                log.debug("Could not find any user.");
            }

            List deliveries_list = deliveries_el.getChildren("Delivery");
            for(int i=0; i<deliveries_list.size();i++){
                Element delivery_el = (Element) deliveries_list.get(i);
                int delivery_id = Integer.parseInt(delivery_el.getAttributeValue("id"));
                String user = delivery_el.getAttributeValue("user");
                log.debug("Delivery "+(i+1)+": id="+delivery_id+", user="+user);
                delivery_id_user_map.put(delivery_id, user);

                Element sns_el = delivery_el.getChild("Sns");
                if (sns_el != null){
                    List sns_list = sns_el.getChildren("Sn");
                    for(int j=0; j<sns_list.size(); j++){
                        Element sn_el = (Element) sns_list.get(j);
                        long service_number = Long.parseLong(sn_el.getValue());
                        service_number_delivery_id_map.put(service_number, delivery_id);
                        log.debug("service number "+(j+1)+": "+service_number+" --> "+delivery_id);
                    }
                } else {
                    log.debug("Couldn't find any service number.");
                }

            }

            Manager.getInstance().setUserPasswordMap(user_password_map);

        } catch(JDOMException e) {
            throw new UpdateConfigurationException(e);
        } catch (IOException e) {
            throw new UpdateConfigurationException(e);
        } catch (SAXException e) {
            throw new UpdateConfigurationException(e);
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

            long send_receipt_time = System.currentTimeMillis();

            Data data = new Data(message_id, send_receipt_time);
            log.debug("l1");
            journal.write(sequence_number, data, Status.SEND);

            table.put(sequence_number, data);
            log.debug("Remember to the memory: " + sequence_number+" --> " + data);

            log.debug("Try to send delivery receipt with message id '"+message_id+"' and sequence number '"+sequence_number);
            smppServer.send(delivery_receipt);
            log.debug("Successfully send delivery receipt with sequence number "+sequence_number);



        }
    }


}

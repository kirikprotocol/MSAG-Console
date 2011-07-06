package mobi.eyeline.dcpgw;

import mobi.eyeline.dcpgw.exeptions.InitializationException;
import mobi.eyeline.dcpgw.exeptions.UpdateConfigurationException;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.delivery.MessageState;
import mobi.eyeline.informer.admin.delivery.changelog.ChangeDeliveryStatusEvent;
import mobi.eyeline.informer.admin.delivery.changelog.ChangeMessageStateEvent;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangeListener;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangesDetectorImpl;
import mobi.eyeline.informer.admin.filesystem.*;
import mobi.eyeline.smpp.api.pdu.Request;
import mobi.eyeline.smpp.api.processing.ProcessingQueue;
import mobi.eyeline.smpp.api.processing.QueueException;
import mobi.eyeline.smpp.api.types.EsmMessageType;
import mobi.eyeline.smpp.api.types.RegDeliveryReceipt;
import org.apache.log4j.Logger;

import java.io.*;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.atomic.AtomicLong;

import mobi.eyeline.smpp.api.SmppServer;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.PDUListener;
import mobi.eyeline.smpp.api.pdu.PDU;
import mobi.eyeline.smpp.api.pdu.Message;
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


/**
 * Created by Serge Lugovoy
 * Date: Jan 22, 2010
 * Time: 5:36:58 PM
 */
public class SMPP2DCPGateway extends Thread implements PDUListener {

    private static Logger log = Logger.getLogger(SMPP2DCPGateway.class);

    private SmppServer smppServer;
    private ProcessingQueue procQueue;

    private HashMap<String, String> user_password_map;
    private HashMap<Integer, String> delivery_id_user_map;
    private HashMap<Long, Integer> service_number_delivery_id_map;

    private String mapping_filename;

    private Schema schema;

    private AtomicLong gateway_mgsId = new AtomicLong(0);

    protected DeliveryChangesDetectorImpl deliveryChangesDetector;

    protected FileSystem fileSystem;

    private SimpleDateFormat sdf = new SimpleDateFormat("yyMMddHHmm");

    public SMPP2DCPGateway() throws SmppException, InitializationException{

        Properties config = new Properties();

        String userDir = System.getProperty("user.dir");
        String filename = userDir+"/config/dcpgw.properties";

        mapping_filename = userDir+"/config/mapping.xml";
        String schema_filename = userDir+"/config/mapping.xsd";

        try {
            config.load(new FileInputStream(filename));
        } catch (IOException e) {
            log.debug("Couldn't load properties file.");
            throw new InitializationException(e);
        }

        Runtime.getRuntime().addShutdownHook(this);

        procQueue = new ProcessingQueue(config,

            new PDUListener() {

                public boolean handlePDU(PDU pdu) {
                    long time = System.currentTimeMillis();
                    long gId = time + gateway_mgsId.incrementAndGet();
                    log.debug("Handle pdu with type "+pdu.getType()+", set gId '"+gId+"'.");
                    switch (pdu.getType()) {

                        case SubmitSM:{

                            Message request = (Message) pdu;

                            Manager.getInstance().setRequest(gId, request, time);

                            Address source_address = request.getSourceAddress();
                            long service_number = Long.parseLong(source_address.getAddress());
                            int delivery_id = service_number_delivery_id_map.get(service_number);
                            String login = delivery_id_user_map.get(delivery_id);

                            log.debug("Try to add SubmitSM with gId '"+gId+"' has: service_number '"+service_number+"', delivery_id '"+delivery_id+"', user '"+login+"'.");
                            Manager.getInstance().getSender(login).addMessage(delivery_id, gId);

                            break;
                        }
                        case DataSM: {

                            Message request = (Message) pdu;

                            Manager.getInstance().setRequest(gId, request, time);

                            Address source_address = request.getSourceAddress();
                            long service_number = Long.parseLong(source_address.getAddress());
                            int delivery_id = service_number_delivery_id_map.get(service_number);
                            String login = delivery_id_user_map.get(delivery_id);

                            log.debug("Try to add DataSM with gId '"+gId+"', service_number '"+service_number+"', delivery_id '"+delivery_id+"', user '"+login+"'.");
                            Manager.getInstance().getSender(login).addMessage(delivery_id, gId);

                            break;
                        }

                        case DeliverSMResp:{
                            Message request = (Message) pdu;

                        }

                    }

                    return true;
                }
            }

        , null);

        smppServer = new SmppServer(config, this);

        Manager.getInstance().setSmppServer(smppServer);

        user_password_map = new HashMap<String, String>();
        delivery_id_user_map = new HashMap<Integer, String>();
        service_number_delivery_id_map = new HashMap<Long, Integer>();

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

        fileSystem = FileSystem.getFSForSingleInst();
        try {
            deliveryChangesDetector = new DeliveryChangesDetectorImpl(new File(userDir+File.separator+"final_log"), fileSystem);

            deliveryChangesDetector.addListener(new DeliveryChangeListener() {
                @Override
                public void messageStateChanged(ChangeMessageStateEvent e) throws AdminException {
                    Properties p = e.getProperties();
                    if (p!=null){
                        if (p.containsKey("gId")){
                            log.debug(e);

                            String s = p.getProperty("gId");
                            log.debug("gId="+s);

                            long gId = Long.parseLong(s);

                            Message request = Manager.getInstance().getRequest(gId);
                            long time = Manager.getInstance().getTime(gId);
                            Calendar c = Calendar.getInstance();
                            c.setTimeInMillis(time);


                            if( request.getRegDeliveryReceipt() != RegDeliveryReceipt.None ) {

                                Message rcpt = request.getAnswer();
                                rcpt.setEsmMessageType(EsmMessageType.DeliveryReceipt);
                                String notf_date = sdf.format(e.getEventDate());
                                String subm_date = sdf.format(c.getTime());

                                MessageState messageState = e.getMessageState();

                                FinalMessageStates finalMessageStates;
                                if (messageState == MessageState.Delivered){
                                    finalMessageStates = FinalMessageStates.DELIVERED;
                                } else if (messageState == MessageState.Expired){
                                    finalMessageStates = FinalMessageStates.EXPIRED;
                                } else {
                                    finalMessageStates = FinalMessageStates.UNKNOWN;
                                }

                                rcpt.setMessage("gId:" + gId + " sub:001 dlvrd:001 submit date:" + subm_date + " done date:" + notf_date + " stat:"+finalMessageStates+" err:000 Text:");
                                try {
                                    smppServer.send(rcpt);
                                } catch (SmppException e1) {
                                    log.error("Couldn't send delivery receipt to the client.", e1);
                                    // todo ?
                                }
                            }

                        }
                    }
                }

                @Override
                public void deliveryStateChanged(ChangeDeliveryStatusEvent e) throws AdminException {
                    log.debug(e);
                }
            });
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
                    log.debug("User �"+(i+1)+": login="+login+" password="+password);
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
                log.debug("Delivery �"+(i+1)+": id="+delivery_id+", user="+user);
                delivery_id_user_map.put(delivery_id, user);

                Element sns_el = delivery_el.getChild("Sns");
                if (sns_el != null){
                    List sns_list = sns_el.getChildren("Sn");
                    for(int j=0; j<sns_list.size(); j++){
                        Element sn_el = (Element) sns_list.get(j);
                        long service_number = Long.parseLong(sn_el.getValue());
                        log.debug("service number �"+(j+1)+": "+service_number);
                        service_number_delivery_id_map.put(service_number, delivery_id);
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
            new SMPP2DCPGateway();
        } catch (SmppException e) {
            log.error(e);
        } catch (InitializationException e) {
            log.error("Couldn't initialize gateway.", e);
        }

    }


}

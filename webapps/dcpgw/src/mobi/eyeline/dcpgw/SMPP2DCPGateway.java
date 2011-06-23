package mobi.eyeline.dcpgw;

import mobi.eyeline.dcpgw.exeptions.InitializationException;
import mobi.eyeline.dcpgw.exeptions.UpdateConfigurationException;
import mobi.eyeline.smpp.api.pdu.Request;
import mobi.eyeline.smpp.api.processing.ProcessingQueue;
import mobi.eyeline.smpp.api.processing.QueueException;
import org.apache.log4j.Logger;

import java.io.*;
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
                    log.debug("Handle pdu with type "+pdu.getType());

                    Long gId = System.currentTimeMillis() + gateway_mgsId.incrementAndGet();

                    switch (pdu.getType()) {

                        case SubmitSM: {

                            Message request = (Message) pdu;

                            Address source_address = request.getSourceAddress();
                            long service_number = Long.parseLong(source_address.getAddress());
                            int delivery_id = service_number_delivery_id_map.get(service_number);
                            String login = delivery_id_user_map.get(delivery_id);
                            String password = user_password_map.get(login);
                            log.debug("service_number: "+service_number+", delivery_id: "+delivery_id+", user: "+login+", password: "+password);

                            Manager.getInstance().getSender(login).addMessage(delivery_id, gId ,request);
                        }

                        case DataSM: {

                            Message request = (Message) pdu;

                            Address source_address = request.getSourceAddress();
                            long service_number = Long.parseLong(source_address.getAddress());
                            int delivery_id = service_number_delivery_id_map.get(service_number);
                            String login = delivery_id_user_map.get(delivery_id);
                            String password = user_password_map.get(login);
                            log.debug("service_number: "+service_number+", delivery_id: "+delivery_id+", user: "+login+", password: "+password);

                            Manager.getInstance().getSender(login).addMessage(delivery_id, gId,  request);
                        }

                    }

                    return false;
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
                    log.debug("User ¹"+(i+1)+": login="+login+" password="+password);
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
                log.debug("Delivery ¹"+(i+1)+": id="+delivery_id+", user="+user);
                delivery_id_user_map.put(delivery_id, user);

                Element sns_el = delivery_el.getChild("Sns");
                if (sns_el != null){
                    List sns_list = sns_el.getChildren("Sn");
                    for(int j=0; j<sns_list.size(); j++){
                        Element sn_el = (Element) sns_list.get(j);
                        long service_number = Long.parseLong(sn_el.getValue());
                        log.debug("service number ¹"+(j+1)+": "+service_number);
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

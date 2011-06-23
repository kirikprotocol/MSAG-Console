package mobi.eyeline.dcpgw;

import mobi.eyeline.smpp.api.SmppServer;
import mobi.eyeline.smpp.api.pdu.Message;
import mobi.eyeline.smpp.api.pdu.Request;
import org.apache.log4j.Logger;

import java.io.*;
import java.util.HashMap;
import java.util.Properties;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 26.05.11
 * Time: 10:07
 */
public class Manager {

    private static Logger log = Logger.getLogger(Manager.class);

    // Use Bill Pugh's singleton - thread-safe singleton with lazy initialization.

    /**
    * SingletonHolder is loaded on the first execution of Singleton.getInstance()
    * or the first access to SingletonHolder.INSTANCE, not before.
    */
    private static class LazyManagerHolder {
        public static Manager singletonInstance = new Manager();
    }

    public static Manager getInstance(){
        return LazyManagerHolder.singletonInstance ;
    }

    private int capacity;

    private long timeout;

    private HashMap<String, Sender> user_senders_map;

    private HashMap<String, String> user_password_map;

    private String informer_host;
    private int informer_port;

    private SmppServer smppServer;

    private HashMap<Long, Request> gId_request_map;

    // Private constructor prevents instantiation from other classes
    private Manager(){
        String userDir = System.getProperty("user.dir");
        String filename = userDir+"/config/dcpgw.properties";

        Properties prop = new Properties();

        try{
            prop.load(new FileInputStream(filename));
        } catch (IOException e) {
            log.error(e);
            System.exit(1);
        }

        String s = prop.getProperty("informer.host");
        if (s != null && !s.isEmpty()){
            informer_host = s;
            log.debug("Set informer host: "+ informer_host);
        } else {
            log.error("informer.host property is invalid or not specified in config");
            System.exit(1);
        }

        s = prop.getProperty("informer.port");
        if (s != null && !s.isEmpty()){
            informer_port = Integer.parseInt(s);
            log.debug("Set informer port: "+ informer_port);
        } else {
            log.error("informer.port property is invalid or not specified in config");
            System.exit(1);
        }

        s = prop.getProperty("informer.messages.list.capacity");
        if (s != null && !s.isEmpty()){
            capacity = Integer.parseInt(s);
            log.debug("Configuration property: informer.messages.list.capacity="+capacity);
        } else {
            log.error("Configuration property 'informer.messages.list.capacity' is invalid or not specified in config");
            System.exit(1);
        }

        s = prop.getProperty("informer.messages.sending.timeout.mls");
        if (s != null && !s.isEmpty()){
            timeout = Integer.parseInt(s);
            log.debug("Configuration property: informer.messages.sending.timeout.mls="+capacity);
        } else {
            log.error("Configuration property 'informer.messages.sending.timeout.mls' is invalid or not specified in config");
            System.exit(1);
        }

        user_senders_map = new HashMap<String, Sender>();

        gId_request_map = new HashMap<Long, Request>();
    }

    public void setSmppServer(SmppServer smppServer){
        this.smppServer = smppServer;
    }

    public void setUserPasswordMap(HashMap<String, String> user_password_map){
        this.user_password_map = user_password_map;
    }

    synchronized public Sender getSender(String user){
        log.debug("Try to get sender for user '"+user+"' ...");

        Sender sender;
        if (user_senders_map.containsKey(user)){
            sender = user_senders_map.get(user);
        } else {
            log.debug("Try to initialize sender for user '"+user+"'.");
            sender = new Sender(informer_host, informer_port, user, user_password_map.get(user), capacity, timeout, smppServer);
            user_senders_map.put(user, sender);
            sender.start();
        }

        return sender;
    }



}

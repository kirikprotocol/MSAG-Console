package mobi.eyeline.dcpgw;

import mobi.eyeline.smpp.api.SmppServer;
import mobi.eyeline.smpp.api.pdu.Message;
import org.apache.log4j.Logger;

import java.io.*;
import java.util.*;

/**
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

    private long sending_timeout;

    private Hashtable<String, Sender> user_senders_map;

    private Hashtable<String, String> user_password_map;

    private String informer_host;
    private int informer_port;

    private SmppServer smppServer;

    private Hashtable<Long, Message> id_rcpt_table;
    private Hashtable<Long, Date> id_submit_time_table;

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

        s = prop.getProperty("sending.timeout.mls");
        if (s != null && !s.isEmpty()){
            sending_timeout = Integer.parseInt(s);
            log.debug("Configuration property: sending.timeout.mls="+sending_timeout);
        } else {
            log.error("Configuration property 'sending.timeout.mls' is invalid or not specified in config");
            System.exit(1);
        }

        user_senders_map = new Hashtable<String, Sender>();

        id_rcpt_table = new Hashtable<Long, Message>();
        id_submit_time_table = new Hashtable<Long, Date>();
    }

    public void setSmppServer(SmppServer smppServer){
        this.smppServer = smppServer;
    }

    public void setUserPasswordMap(Hashtable<String, String> user_password_map){
        this.user_password_map = user_password_map;
    }

    synchronized public Sender getSender(String user){
        //log.debug("Try to get sender for user '"+user+"' ...");

        Sender sender;
        if (user_senders_map.containsKey(user)){
            sender = user_senders_map.get(user);
        } else {
            log.debug("Try to initialize sender for user '"+user+"'.");
            sender = new Sender(informer_host, informer_port, user, user_password_map.get(user), capacity, sending_timeout, smppServer);
            user_senders_map.put(user, sender);
            sender.start();
        }

        return sender;
    }

    public void rememberReceiptMessage(long message_id, Message rcpt){
        id_rcpt_table.put(message_id, rcpt);
    }

    public Message getReceiptMessage(long message_id){
        return id_rcpt_table.get(message_id);
    }

    public Date getSubmitTime(long message_id){
        return id_submit_time_table.get(message_id);
    }

    public void rememberSubmitTime(long message_id, Date date){
        id_submit_time_table.put(message_id, date);
    }

}

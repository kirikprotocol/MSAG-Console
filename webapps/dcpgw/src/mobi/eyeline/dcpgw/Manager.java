package mobi.eyeline.dcpgw;

import org.apache.log4j.Logger;

import java.io.*;
import java.util.HashMap;
import java.util.Properties;
import java.util.concurrent.LinkedBlockingQueue;
import mobi.eyeline.informer.admin.delivery.Message;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 26.05.11
 * Time: 10:07
 */
public class Manager {

    private static Logger log = Logger.getLogger(Manager.class);

    private static Manager instance;

    /*  BlockingQueue implementations are thread-safe. All queuing methods achieve their effects atomically
        using internal locks or other forms of concurrency control. However, the bulk Collection operations
        addAll, containsAll, retainAll and removeAll are not necessarily performed atomically unless specified
        otherwise in an implementation. So it is possible, for example, for addAll(c) to fail (throwing an exception)
        after adding only some of the elements in c."
     */
    private LinkedBlockingQueue<Message> queue;

    private int capacity;

    private long timeout;

    private HashMap<String, Sender> user_senders_map;

    private HashMap<String, String> user_password_map;
    private HashMap<Integer, String> delivery_id_user_map;
    private HashMap<Long, Integer> service_number_delivery_id_map;

    private String informer_host;
    private int informer_port;

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
    }

    public void setUserPasswordMap(HashMap<String, String> user_password_map){
        this.user_password_map = user_password_map;
    }

    public void setDeliveryIdUserMap(HashMap<Integer, String> delivery_id_user_map){
        this.delivery_id_user_map = delivery_id_user_map;
    }

    public void setServiceNumberDeliveryIdMap(HashMap<Long, Integer> service_number_delivery_id_map){
        this.service_number_delivery_id_map = service_number_delivery_id_map;
    }

    public static synchronized Manager getInstance(){
        if (instance == null){
            instance = new Manager();
        }
        return instance;
    }

    synchronized public void addMessage(String user, int delivery_id, Message message){
        log.debug("Try to add message to informer messages queue ...");

        Sender sender;
        if (user_senders_map.containsKey(user)){
            sender = user_senders_map.get(user);
        } else{
            sender = new Sender(informer_host, informer_port, user, user_password_map.get(user), capacity, timeout);
            user_senders_map.put(user, sender);
            sender.start();
        }

        sender.addMessage(delivery_id, message);
    }

/*    public Data poll(){
        log.debug("Manager.poll() ...");
        *//*
           E = LinkedBlockingQueue<E>.pool()
           Retrieves and removes the head of this queue, or returns null if this queue is empty.
        *//*
        return queue.poll();
    }

    public Data peek(){
        //log.debug("Manager.peek() ...");
        *//*
            E = LinkedBlockingQueue<E>.pool()
            Retrieves, but does not remove, the head of this queue, or returns null if this queue is empty.
         *//*
        return queue.peek();
    }

    public boolean isEmpty(){
        //log.debug("Manager.isEmpty() ...");
        return queue.isEmpty();
    }

    public boolean remove(Data data){
        log.debug("Manager.remove() ...");
        return queue.remove(data);
    }

    synchronized public boolean sendMail(){
        log.debug("Check whether the message queue ...");

        while(com.eyeline.services.sombel.rtasks.Manager.getInstance().isEmpty()){
            try {
                log.debug("Mail queue is empty, wait ...");
                wait();
            } catch (InterruptedException e) {
                log.error(e);
            }
        }

        if (!com.eyeline.services.sombel.rtasks.Manager.getInstance().isEmpty()){

            log.debug("Mail queue is not empty, try to send mail ...");
            Data data = com.eyeline.services.sombel.rtasks.Manager.getInstance().poll();

            if (data != null){
                try{
                    String msisdn = data.getMsisdn();
                    Date date = data.getDate();

                    Properties props = new Properties();
                    Session session = Session.getDefaultInstance(props, null);
                    MimeMessage message = new MimeMessage(session);
                    message.setSubject(email_subject);

                    SimpleDateFormat email_sdf = new SimpleDateFormat("dd.MM.yy HH:mm");
                    String email_date = email_sdf.format(date);
                    log.debug("Formatted email date: "+email_date);
                    String a = msisdn.startsWith("+")? msisdn : "+"+msisdn;
                    String[] args = {a, email_date};
                    MessageFormat form = new MessageFormat(email_message_format);
                    String email_text = form.format(args);
                    log.debug("Formatted email text: "+email_text);
                    message.setText(email_text);

                    Address sender = new InternetAddress(email_sender, email_sender_name);
                    for(String s: email_recipients){
                        log.debug("Recipient: "+s.trim());
                        Address recipient = new InternetAddress(s.trim());
                        message.addRecipient(Message.RecipientType.TO, recipient);
                    }
                    message.setFrom(sender);

                    Transport.send(message);
                    log.debug("Successfully send mail!");

                } catch (MessagingException e){
                    log.error(e);
                } catch (UnsupportedEncodingException e){
                    log.error(e);
                }

            } else {
                log.warn("Poll method return null from not empty queue.");
            }
        } else {
            log.warn("Mail queue is empty ...");
        }

        return com.eyeline.services.sombel.rtasks.Manager.getInstance().isEmpty();
    }*/


}

package mobi.eyeline.dcpgw;

import org.apache.log4j.Logger;

import java.io.*;
import java.text.MessageFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Properties;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 26.05.11
 * Time: 10:07
 */
public class InformerMessagesQueue {

    private static Logger log = Logger.getLogger(InformerMessagesQueue.class);

    private static InformerMessagesQueue instance;

    /*  BlockingQueue implementations are thread-safe. All queuing methods achieve their effects atomically
        using internal locks or other forms of concurrency control. However, the bulk Collection operations
        addAll, containsAll, retainAll and removeAll are not necessarily performed atomically unless specified
        otherwise in an implementation. So it is possible, for example, for addAll(c) to fail (throwing an exception)
        after adding only some of the elements in c."
     */
    private LinkedBlockingQueue<mobi.eyeline.informer.admin.delivery.Message> queue;

    private String config_file_encoding = "windows-1251";

    private InformerMessagesQueue(){

        String userDir = System.getProperty("user.dir");
        String filename = userDir+"/config/sombel.txt";

        Properties prop = new Properties();
        int capacity;

        try{
            BufferedReader in = new BufferedReader(
                    new InputStreamReader(
                            new FileInputStream(filename), config_file_encoding
                    )
            );
            prop.load(in);
        } catch (IOException e) {
            log.error(e);
        }
    }

    public static synchronized InformerMessagesQueue getInstance(){
        if (instance == null){
            instance = new InformerMessagesQueue();
        }
        return instance;
    }

    synchronized public boolean addInformerMessage(mobi.eyeline.informer.admin.delivery.Message data){
        /*
           LinkedBlockingQueue<E>.offer(E e)
           Inserts the specified element at the tail of this queue if it is possible to do so immediately without
           exceeding the queue's capacity, returning true upon success and false if this queue is full.
         */

        boolean add = queue.offer(data);

        log.debug("Added message to the queue, try to notify MailSender ...");
        notify();

        return add;
    }

/*    public Data poll(){
        log.debug("InformerMessagesQueue.poll() ...");
        *//*
           E = LinkedBlockingQueue<E>.pool()
           Retrieves and removes the head of this queue, or returns null if this queue is empty.
        *//*
        return queue.poll();
    }

    public Data peek(){
        //log.debug("InformerMessagesQueue.peek() ...");
        *//*
            E = LinkedBlockingQueue<E>.pool()
            Retrieves, but does not remove, the head of this queue, or returns null if this queue is empty.
         *//*
        return queue.peek();
    }

    public boolean isEmpty(){
        //log.debug("InformerMessagesQueue.isEmpty() ...");
        return queue.isEmpty();
    }

    public boolean remove(Data data){
        log.debug("InformerMessagesQueue.remove() ...");
        return queue.remove(data);
    }

    synchronized public boolean sendMail(){
        log.debug("Check whether the message queue ...");

        while(com.eyeline.services.sombel.rtasks.InformerMessagesQueue.getInstance().isEmpty()){
            try {
                log.debug("Mail queue is empty, wait ...");
                wait();
            } catch (InterruptedException e) {
                log.error(e);
            }
        }

        if (!com.eyeline.services.sombel.rtasks.InformerMessagesQueue.getInstance().isEmpty()){

            log.debug("Mail queue is not empty, try to send mail ...");
            Data data = com.eyeline.services.sombel.rtasks.InformerMessagesQueue.getInstance().poll();

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

        return com.eyeline.services.sombel.rtasks.InformerMessagesQueue.getInstance().isEmpty();
    }*/


}

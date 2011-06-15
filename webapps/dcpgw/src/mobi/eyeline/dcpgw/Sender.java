package mobi.eyeline.dcpgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DcpConnection;
import mobi.eyeline.informer.admin.delivery.Message;
import org.apache.log4j.Logger;

import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 14.06.11
 * Time: 18:02
 */
public class Sender extends Thread{

    private static Logger log = Logger.getLogger(Sender.class);

    private DcpConnection connection;
    private String host;
    private int port;
    private String login;
    private String password;

    private int capacity;

    private long timeout;

    private HashMap<Integer, List<Message>> lists_map;

    private boolean interrupt = false;

    public Sender(String host, int port, final String login, String password, int capacity, long timeout){

        this.host = host;
        this.port = port;
        this.login = login;
        this.password = password;

        this.capacity = capacity;

        this.timeout = timeout;

        lists_map = new HashMap<Integer, List<Message>>();
    }

    public void addMessage(int delivery_id, Message message){
        log.debug("Try to add message to the list for delivery with identifier "+delivery_id+" ...");
        List<Message> list = lists_map.get(delivery_id);
        if (list == null){
            log.debug("Appropriate list not found.");
            list = Collections.synchronizedList(new LinkedList<Message>());
            lists_map.put(delivery_id, list);
            log.debug("Initialize new list for delivery with identifier "+delivery_id+".");
        }

        list.add(message);
        log.debug("Successfully add messages to list.");

        int size = list.size();
        log.debug("Queue size has increased to "+size+".");

        if (size == capacity) {
            notify();
            log.debug("Queue is full, try to notify the flow to add messages to informer's deliveries ...");
        } else {
            log.debug("Queue size less than capacity");
        }
    }

    public void run() {
        while(!interrupt){

            for (Map.Entry<Integer, List<Message>> entry : lists_map.entrySet()) {
                int delivery_id = entry.getKey();
                List<Message> list = entry.getValue();
                if (list.size() == capacity) {

                    if (connection == null){
                        try {
                            connection = new DcpConnection(host, port, login, password);
                        } catch (AdminException e) {
                            log.error(e);
                        }
                    }

                    try {
                        connection.addDeliveryMessages(delivery_id, list);
                    } catch (AdminException e) {
                        log.error(e);
                    }

                }
            }

            try {
                wait(timeout);
            } catch (InterruptedException e) {
                log.error(e);
            }
        }

    }

    public void interrupt(){
        interrupt = true;
    }



}

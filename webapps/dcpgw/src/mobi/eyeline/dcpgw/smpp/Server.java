package mobi.eyeline.dcpgw.smpp;

import mobi.eyeline.dcpgw.Config;
import mobi.eyeline.dcpgw.exeptions.InitializationException;
import mobi.eyeline.dcpgw.journal.Data;
import mobi.eyeline.smpp.api.*;
import mobi.eyeline.smpp.api.pdu.DeliverSMResp;
import mobi.eyeline.smpp.api.pdu.PDU;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.*;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 08.09.11
 * Time: 9:58
 */
public class Server{

    private static Logger log = Logger.getLogger(Server.class);

    private static final String CONNECTION_PREFIX = "smpp.sme.";

    private static Server instance = new Server();

    private SubSmppServer server;

    private Hashtable<String, Connection> connections;

    private HashSet<String> deleted_connections;

    private Properties properties;

    private int initial_receipt_sn_rang;
    private long limit;
    private long rang;
    private File receipts_rang_file;

    private static AtomicInteger ai = new AtomicInteger(0);

    public static Server getInstance(){
        return instance;
    }

    public void init(Properties properties, PDUListener listener) throws SmppException, InitializationException {
        this.properties = properties;
        server = new SubSmppServer(properties, listener);

        // Initialize smpp connections.
        connections = new Hashtable<String, Connection>();
        for (Object skey : properties.keySet()) {
            String key = (String) skey;
            if (key.startsWith(CONNECTION_PREFIX) && key.endsWith(".password")) {
                String name = key.substring(CONNECTION_PREFIX.length());
                name = name.substring(0, name.length() - ".password".length());

                int speed = Integer.parseInt((String) properties.get(CONNECTION_PREFIX+name+"send.receipts.speed"));
                int max_time = Integer.parseInt((String) properties.get(CONNECTION_PREFIX+name+"send.receipt.max.time.min"));

                connections.put(name, new Connection(name, speed, max_time));
                log.debug("initialize smpp connection "+name);
            }
        }

        deleted_connections = new HashSet<String>();

        Config config = Config.getInstance();
        initial_receipt_sn_rang = config.getReceiptsSequenceNumberRang();
        rang = config.getRang();
        limit = initial_receipt_sn_rang + rang;
        receipts_rang_file = config.getReceiptsRangFile();
    }

    public synchronized int getReceiptSequenceNumber(){
        int sn = initial_receipt_sn_rang + ai.incrementAndGet();
        if (sn == limit){
            try {
                PrintWriter pw = new PrintWriter(new FileWriter(receipts_rang_file));
                limit = limit + rang;
                pw.println(limit);
                pw.flush();
                pw.close();
            } catch (IOException e) {
                log.error("Couldn't write to file new initial message id rang.", e);
            }
        }
        return sn;
    }

    public void send(PDU resp) throws SmppException {
        server.send(resp, false);
    }

    public void update(Properties new_properties) throws SmppException {
        // Initialize new connections
        for (Object skey : new_properties.keySet()) {
            String key = (String) skey;
            if (key.startsWith(CONNECTION_PREFIX) && key.endsWith(".password")) {
                if (!properties.containsKey(key)){
                    String name = key.substring(CONNECTION_PREFIX.length());
                    name = name.substring(0, name.length() - ".password".length());

                    int speed = Integer.parseInt((String) properties.get(CONNECTION_PREFIX+name+"send.receipts.speed"));
                    int max_time = Integer.parseInt((String) properties.get(CONNECTION_PREFIX+name+"send.receipt.max.time.min"));

                    connections.put(name, new Connection(name, speed, max_time));
                    log.debug("Initialize new smpp connection "+name);
                }
            }
        }

        // Update parameters for existing connection
        for (Object skey : new_properties.keySet()) {
            String key = (String) skey;
            if (key.startsWith(CONNECTION_PREFIX) && key.endsWith(".password")) {
                if (properties.containsKey(key)){
                    String name = key.substring(CONNECTION_PREFIX.length());
                    name = name.substring(0, name.length() - ".password".length());

                    int speed = Integer.parseInt((String) properties.get(CONNECTION_PREFIX+name+"send.receipts.speed"));
                    int max_time = Integer.parseInt((String) properties.get(CONNECTION_PREFIX+name+"send.receipt.max.time.min"));

                    Connection connection = connections.get(name);

                    connection.setSendReceiptsSpeed(speed);
                    connection.setSendReceiptMaxTimeout(max_time);

                    log.debug("Update parameters for smpp connection "+name);
                }
            }
        }

        // Remove deleted connections.
        for (Object skey : properties.keySet()) {
            String key = (String) skey;
            if (key.startsWith(CONNECTION_PREFIX) && key.endsWith(".password")) {
                if (!new_properties.containsKey(key)){
                    String name = key.substring(CONNECTION_PREFIX.length());
                    name = name.substring(0, name.length() - ".password".length());
                    deleted_connections.add(name);
                    Connection connection = connections.remove(name);
                    connection.close();
                    log.debug("Remove deleted smpp connection "+name);
                }
            }
        }

        properties = new_properties;

        server.update(new_properties);
    }

    public void shutdown(){
        server.shutdown();
    }

    public void send(Data data){
        String connection_name = data.getConnectionName();

        Connection connection = connections.get(connection_name);
        if (connection != null){

            connection.send(data);
        } else {

            if (!deleted_connections.contains(connection_name)) {
                log.error("Couldn't send DeliverSM with message id "+data.getMessageId()+", couldn't find smpp connection "+connection_name);
            } else {
                log.debug("Couldn't send DeliverSM with message id "+data.getMessageId()+", connection "+connection_name+" was deleted ot disabled.");
            }

        }

    }

    public boolean handle(DeliverSMResp resp){
        String connection_name = resp.getConnectionName();
        Connection connection = connections.get(connection_name);
        if (connection != null){
            connection.handle(resp);
            return true;
        } else {

            if (!deleted_connections.contains(connection_name)) {
                log.error("Couldn't handle DeliverSMResp with message id "+resp.getMessageId()+" and sequence number "+
                        resp.getSequenceNumber()+", couldn't find smpp connection "+connection_name);
            } else {
                log.debug("Couldn't handle DeliverSMResp with message_id "+resp.getMessageId()+" and sequence number "+
                        resp.getSequenceNumber()+", connection "+connection_name+" was deleted or disabled.");
            }
            return false;
        }
    }

    private class SubSmppServer extends SmppServer{

        public SubSmppServer(Properties config, PDUListener listener) throws SmppException {
            super(config, listener);
        }

        public void update(Properties config) throws SmppException {
            configure(config);
        }

    }

    public Connection getConnection(String name){
        return connections.get(name);
    }

}

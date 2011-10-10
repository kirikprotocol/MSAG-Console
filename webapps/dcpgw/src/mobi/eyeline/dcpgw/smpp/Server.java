package mobi.eyeline.dcpgw.smpp;

import mobi.eyeline.dcpgw.exeptions.CouldNotWriteToJournalException;
import mobi.eyeline.dcpgw.exeptions.InitializationException;
import mobi.eyeline.dcpgw.journal.Data;
import mobi.eyeline.smpp.api.*;
import mobi.eyeline.smpp.api.pdu.DeliverSMResp;
import mobi.eyeline.smpp.api.pdu.PDU;
import org.apache.log4j.Logger;

import java.util.*;

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
                connections.put(name, new Connection(name));
                log.debug("initialize smpp connection "+name);
            }
        }

        deleted_connections = new HashSet<String>();
    }

    /**
     * Used for sending SubmitSMResp and DataSMResp to smpp client.
     * @param resp
     * @throws SmppException
     */
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
                    connections.put(name, new Connection(name));
                    log.debug("Initialize new smpp connection "+name);
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
                log.error("Couldn't handle DeliverSMResp with message id "+resp.getMessageId()+" and sequence number "+resp.getSequenceNumber()+", couldn't find smpp connection "+connection_name);
            } else {
                log.debug("Couldn't handle DeliverSMResp with message_id "+resp.getMessageId()+" and sequence number "+resp.getSequenceNumber()+", connection "+connection_name+" was deleted or disabled.");
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

}

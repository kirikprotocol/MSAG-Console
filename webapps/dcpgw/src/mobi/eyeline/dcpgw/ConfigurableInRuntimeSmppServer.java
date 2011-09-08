package mobi.eyeline.dcpgw;

import mobi.eyeline.smpp.api.PDUListener;
import mobi.eyeline.smpp.api.ServerSession;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.SmppServer;
import org.apache.log4j.Logger;

import java.util.Hashtable;
import java.util.Map;
import java.util.Properties;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 08.09.11
 * Time: 9:58
 */
public class ConfigurableInRuntimeSmppServer extends SmppServer {

    private static Logger log = Logger.getLogger(Gateway.class);

    public ConfigurableInRuntimeSmppServer(Properties config, PDUListener listener) throws SmppException {
        super(config, listener);
    }

    public void updateConnections(Hashtable<String, String> systemId_password_table){
        log.debug("Try to update connections ...");
        for (Map.Entry<String, String> entry : systemId_password_table.entrySet()) {
            String systemId = entry.getKey();
            String password = entry.getValue();
            String connection_property_str = CONNECTION_PREFIX+systemId+".password";

            if (config.contains(connection_property_str)){
                String password_old = config.getProperty(connection_property_str);

                if (!password_old.equals(password)){
                    config.setProperty(connection_property_str, password);
                    log.debug("Set new password for connection with system id '"+systemId+"'.");

                    ServerSession server_session = getSession(systemId);
                    if (server_session != null){
                        server_session.close();
                        log.debug("Close server session for connection with system id '"+systemId+"'.");
                    }
                }
            } else {
                config.setProperty(connection_property_str, password);
                log.debug("Add connection with system id '"+systemId+"' to the smpp server configuration.");
            }
        }

        for (Object skey : config.keySet()) {
            String key = (String) skey;
            if (key.startsWith(CONNECTION_PREFIX) && key.endsWith(".password")) {
                String systemId = key.substring(CONNECTION_PREFIX.length());
                systemId = systemId.substring(0, systemId.length() - ".password".length());

                if (!systemId_password_table.contains(systemId)){
                    log.debug("Detected that new config doesn't contain connection with systemId '"+systemId+"'.");

                    ServerSession server_session = getSession(systemId);
                    if (server_session != null){
                        server_session.close();
                        log.debug("Close server session for connection with system id '"+systemId+"'.");
                    }
                }
            }
        }
        log.debug("Successfully update connections!");
    }


}

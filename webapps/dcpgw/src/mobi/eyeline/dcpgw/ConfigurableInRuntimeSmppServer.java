package mobi.eyeline.dcpgw;

import mobi.eyeline.smpp.api.PDUListener;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.SmppServer;

import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 08.09.11
 * Time: 9:58
 */
public class ConfigurableInRuntimeSmppServer extends SmppServer {

    public ConfigurableInRuntimeSmppServer(Properties config, PDUListener listener) throws SmppException {
        super(config, listener);
    }

    public void update(Properties config) throws SmppException {
        configure(config);
    }

}

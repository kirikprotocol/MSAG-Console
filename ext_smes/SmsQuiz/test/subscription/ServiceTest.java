package subscription;

import com.eyeline.sme.smpp.SMPPTransceiver;
import com.eyeline.sme.handler.MessageHandler;
import com.eyeline.sme.handler.HandlerException;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.ConfigException;

import java.io.File;

import ru.aurorisoft.smpp.SMPPException;
import mobi.eyeline.smsquiz.subscription.manager.SubscriptionManager;
import mobi.eyeline.smsquiz.subscription.manager.SubManagerException;
import mobi.eyeline.smsquiz.storage.StorageException;
import mobi.eyeline.smsquiz.subscription.datasource.ConnectionPoolFactory;
import static org.junit.Assert.assertTrue;

/**
 * author: alkhal
 */

public class ServiceTest {
    public static void init() {

        try {
            ConnectionPoolFactory.init("conf/config.xml");
        } catch (StorageException e) {
            e.printStackTrace();
            assertTrue(false);
        }
        try {
            SubscriptionManager.init();
        } catch (SubManagerException e) {
            e.printStackTrace();
        }

    }
    public static void main(String[] args) {
      init();
      try{
          PropertiesConfig cfg = new PropertiesConfig();
          cfg.load(new File("conf/smpp.properties"));
          final SMPPTransceiver transceiver = new SMPPTransceiver(cfg, "");
          transceiver.connect();
          MessageHandler mh = new MessageHandler("conf/config.xml",transceiver.getInQueue(),transceiver.getOutQueue());
          mh.start();
      } catch (SMPPException e) {
          e.printStackTrace();
      } catch (ConfigException e) {
          e.printStackTrace();
      } catch (HandlerException e) {
          e.printStackTrace();
      }

    }
}

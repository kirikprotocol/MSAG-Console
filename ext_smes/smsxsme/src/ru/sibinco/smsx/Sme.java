package ru.sibinco.smsx;

import com.eyeline.sme.handler.MessageHandler;
import com.eyeline.sme.smpp.SMPPTransceiver;
import com.eyeline.sme.smpp.test.TestMultiplexor;
import com.eyeline.sme.smpp.test.SimpleResponse;
import com.eyeline.utils.config.properties.PropertiesConfig;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.SMPPException;
import ru.aurorisoft.smpp.PDU;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.smsx.engine.soaphandler.blacklist.BlacklistSoapFactory;
import ru.sibinco.smsx.engine.soaphandler.smsxsender.SmsXSenderFactory;
import ru.sibinco.smsx.engine.service.ServiceManager;
import ru.sibinco.smsx.network.advertising.AdvertisingClientFactory;
import ru.sibinco.smsx.network.advertising.AdvertisingClient;
import ru.sibinco.smsx.network.dbconnection.ConnectionPoolFactory;
import ru.sibinco.smsx.network.personalization.PersonalizationClientPoolFactory;

/**
 * User: artem
 * Date: 03.07.2007
 */

public class Sme {

  private static final Category log = Category.getInstance("SME");

  private SMPPTransceiver transceiver;
  private MessageHandler handler;
  private AdvertisingClient senderAdvertisingClient;

  public Sme(String configDir, boolean testMode) throws SmeException {

    try {
      // Init DB connection pool
      ConnectionPoolFactory.init(configDir);

      // Init personalization clients factory
      PersonalizationClientPoolFactory.init(configDir);

      // Init advertising clients factory
      AdvertisingClientFactory.init(configDir);

      // Init context
      Context.init();

      // Init SMPP multiplexor
      if (testMode) {
        System.out.println("SMSX started in test mode");
        transceiver = new SMPPTransceiver(new Multiplexor(), new PropertiesConfig("conf/smpp.properties"), "");
      } else
        transceiver = new SMPPTransceiver(new PropertiesConfig("conf/smpp.properties"), "");

      handler = new MessageHandler("conf/handler.xml", transceiver.getInQueue(), transceiver.getOutQueue());

      // Init services
      ServiceManager.init(configDir, transceiver.getOutQueue());
      ServiceManager.getInstance().startServices();

      // Init SOAP
      BlacklistSoapFactory.init(configDir);
      senderAdvertisingClient = AdvertisingClientFactory.createAdvertisingClient();
      senderAdvertisingClient.connect();
      SmsXSenderFactory.init(configDir, senderAdvertisingClient);

      transceiver.connect();
      handler.start();

    } catch (Throwable e) {
      log.error(e,e);
      throw new SmeException(e);
    }
  }

  public void release() throws SmeException {
    log.error("Stopping smpp multiplexor...");
    try {
      transceiver.shutdown();
    } catch (SMPPException e) {
      throw new SmeException(e);
    } catch (InterruptedException e) {
      throw new SmeException(e);
    }

    try {
      handler.shutdown();
    } catch (InterruptedException e) {
      throw new SmeException(e);
    }

    log.error("Stopping sme engine...");
    log.info("Stopping services...");
    ServiceManager.getInstance().stopServices();
    log.info("Services stopped.");

    log.info("Close sender advertising client...");
    senderAdvertisingClient.close();
    log.info("Sender advertising client closed.");

    log.info("Stopping reload config executor...");
    Context.getInstance().shutdown();
    log.info("Reload config executor stopped.");
  }

  private static class Multiplexor extends TestMultiplexor {

    public void sendResponse(PDU pdu) {
    }

    protected void _sendMessage(Message message) throws SMPPException {
      SimpleResponse r = new SimpleResponse(message);
      r.setStatusClass(Message.STATUS_CLASS_NO_ERROR);
      handleResponse(r);
    }
  }
}

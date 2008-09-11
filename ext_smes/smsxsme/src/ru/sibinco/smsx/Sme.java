package ru.sibinco.smsx;

import com.eyeline.sme.handler.MessageHandler;
import com.eyeline.sme.smpp.SMPPTransceiver;
import com.eyeline.sme.smpp.test.TestMultiplexor;
import com.eyeline.sme.smpp.test.SimpleResponse;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.jmx.log4j.LoggingMBean;
import com.sun.jdmk.comm.HtmlAdaptorServer;
import com.sun.jdmk.comm.AuthInfo;
import org.apache.log4j.Category;
import org.apache.log4j.LogManager;
import ru.aurorisoft.smpp.SMPPException;
import ru.aurorisoft.smpp.PDU;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.smsx.engine.soaphandler.blacklist.BlacklistSoapFactory;
import ru.sibinco.smsx.engine.soaphandler.smsxsender.SmsXSenderFactory;
import ru.sibinco.smsx.engine.soaphandler.groupsend.GroupSendFactory;
import ru.sibinco.smsx.engine.soaphandler.groupedit.GroupEditFactory;
import ru.sibinco.smsx.engine.soaphandler.smsxsubscription.SmsXSubscriptionFactory;
import ru.sibinco.smsx.engine.service.Services;
import ru.sibinco.smsx.engine.service.ServiceManagerMBean;
import ru.sibinco.smsx.network.advertising.AdvertisingClientFactory;
import ru.sibinco.smsx.network.advertising.AdvertisingClient;
import ru.sibinco.smsx.network.dbconnection.ConnectionPoolFactory;
import ru.sibinco.smsx.network.personalization.PersonalizationClientPoolFactory;

import javax.management.MBeanServer;
import javax.management.ObjectName;
import java.io.File;
import java.lang.management.ManagementFactory;

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
      PropertiesConfig smppProperties = new PropertiesConfig();
      smppProperties.load(new File("conf/smpp.properties"));
      if (testMode) {
        System.out.println("SMSX started in test mode");
        transceiver = new SMPPTransceiver(new Multiplexor(), smppProperties, "");
      } else
        transceiver = new SMPPTransceiver(smppProperties, "");

      handler = new MessageHandler("conf/smpphandlers/handler.xml", transceiver.getInQueue(), transceiver.getOutQueue());

      // Init services
      final XmlConfig config = new XmlConfig();
      config.load(new File(configDir, "config.xml"));      
      Services.init(config, transceiver.getOutQueue());
      Services.getInstance().startServices();

      // Init SOAP
      BlacklistSoapFactory.init(configDir);
      senderAdvertisingClient = AdvertisingClientFactory.createAdvertisingClient();
      senderAdvertisingClient.connect();
      SmsXSenderFactory.init(configDir, senderAdvertisingClient);
      GroupSendFactory.init(configDir);
      GroupEditFactory.init(configDir);
      SmsXSubscriptionFactory.init(configDir);

      transceiver.connect();
      handler.start();


      if (config.getSection("jmx") != null) {
        System.out.println("SMSX started in JMX mode");
        int jmxPort = config.getSection("jmx").getInt("port");
        String user = config.getSection("jmx").getString("user");
        String password = config.getSection("jmx").getString("password");
        final MBeanServer mbs = ManagementFactory.getPlatformMBeanServer();
        // SMPPTransceiver MBeans
        mbs.registerMBean(transceiver.getInQueueMonitor(), new ObjectName("SMSX.smpp:mbean=inQueue"));
        mbs.registerMBean(transceiver.getOutQueueMonitor(), new ObjectName("SMSX.smpp:mbean=outQueue"));

        //MessageHandler MBeans
        mbs.registerMBean(handler.getHandlerMBean(), new ObjectName("SMSX.sme:mbean=handler"));

        final LoggingMBean lb = new LoggingMBean("SMSX", LogManager.getLoggerRepository());
        mbs.registerMBean(lb, new ObjectName("SMSX:mbean=logging"));

        final ServiceManagerMBean servicesMBean = Services.getInstance().getMBean("SMSX");
        mbs.registerMBean(servicesMBean, new ObjectName("SMSX:mbean=services"));

        // Load JMX configuration        
        HtmlAdaptorServer adapter = new HtmlAdaptorServer(jmxPort, new AuthInfo[] {new AuthInfo(user, password)});
        mbs.registerMBean(adapter, new ObjectName("SMSX:mbean=htmlAdaptor"));
        adapter.start();
      }

    } catch (Throwable e) {
      e.printStackTrace();
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
    Services.getInstance().stopServices();
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

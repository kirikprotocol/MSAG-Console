package com.eyeline.sponsored;

import com.eyeline.sme.handler.MessageHandler;
import com.eyeline.sme.smpp.SMPPTransceiver;
import com.eyeline.sme.smpp.test.SimpleResponse;
import com.eyeline.sme.smpp.test.TestMultiplexor;
import com.eyeline.sponsored.config.Config;
import com.eyeline.sponsored.distribution.advert.distr.DistributionSme;
import com.eyeline.sponsored.subscription.service.SubscriptionSme;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.Arguments;
import com.eyeline.utils.jmx.log4j.LoggingMBean;
import com.logica.smpp.Data;
import com.sun.jdmk.comm.AuthInfo;
import com.sun.jdmk.comm.HtmlAdaptorServer;
import org.apache.log4j.Category;
import org.apache.log4j.LogManager;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesList;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesListException;

import javax.management.*;
import java.io.File;
import java.util.concurrent.*;
import java.lang.management.ManagementFactory;

/**
 * User: artem
 * Date: 01.02.2008
 */

public class Sme {


  public static void main(String[] args) {

    MessageHandler handler;
    SMPPTransceiver smppTranceiver;
    SubscriptionSme subscriptionSme = null;
    DistributionSme distributionSme = null;

    try {
      final Arguments startArgs = new Arguments(args);

      final XmlConfig xmlConfig = new XmlConfig();
      xmlConfig.load(new File("conf/config.xml"));

      final Config conf = new Config(xmlConfig);

      final SmscTimezonesList timezones = new SmscTimezonesList();
      timezones.load(conf.getTimezonesFile(), conf.getRoutesFile());

      final PropertiesConfig smppProps = new PropertiesConfig();
      smppProps.load(new File(conf.getSmppConfigFile()));

      // Check test mode
      if (startArgs.containsAttr("-t"))
        System.out.println("Sponsored started in test mode");

      smppTranceiver = startArgs.containsAttr("-t") ? new SMPPTransceiver(new Multiplexor(), smppProps, "") : new SMPPTransceiver(smppProps, "");

      subscriptionSme = new SubscriptionSme(xmlConfig, timezones);
      distributionSme = new DistributionSme(xmlConfig, timezones, smppTranceiver.getOutQueue());

      handler = new MessageHandler(conf.getHandlerConfigFile(), smppTranceiver.getInQueue(), smppTranceiver.getOutQueue());

      smppTranceiver.connect();
      handler.start();

      // Check JMX mode
      if (startArgs.containsAttr("-jmx"))
        startJMX(xmlConfig, smppTranceiver, distributionSme, handler);

      final ScheduledExecutorService configReloader = startConfigReloader(timezones, conf);

      Runtime.getRuntime().addShutdownHook(new ShutdownHook(subscriptionSme, distributionSme, configReloader));

    } catch (Throwable e) {
      e.printStackTrace();
      if (subscriptionSme != null)
        subscriptionSme.stop();
      if (distributionSme != null)
        distributionSme.stop();
    }
  }

  private static ScheduledExecutorService startConfigReloader(SmscTimezonesList timezones, Config conf) {
    final ScheduledExecutorService configReloader = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
      public Thread newThread(Runnable r) {
        return new Thread(r, "ConfigReloader");
      }
    });
    configReloader.scheduleAtFixedRate(new ConfigReloadTask(timezones, conf.getTimezonesFile(), conf.getRoutesFile()), 600, 600, TimeUnit.SECONDS);
    return configReloader;
  }

  private static void startJMX(XmlConfig xmlConfig, SMPPTransceiver smppTranceiver, DistributionSme distrSme, MessageHandler handler) throws Exception {
    System.out.println("Sponsored started in JMX mode");
    final MBeanServer mbs = ManagementFactory.getPlatformMBeanServer();
    // SMPPTransceiver MBeans
    mbs.registerMBean(smppTranceiver.getInQueueMonitor(), new ObjectName("Sponsored:mbean=inQueue"));
    mbs.registerMBean(smppTranceiver.getOutQueueMonitor(), new ObjectName("Sponsored:mbean=outQueue"));

    //MessageHandler MBeans
    mbs.registerMBean(handler.getHandlerMBean(), new ObjectName("Sponsored:mbean=handler"));

    //DS MBeans
    mbs.registerMBean(distrSme.getBannerMapMBean(), new ObjectName("Sponsored:mbean=bannerMap"));

    //Distr engine MBean
    DynamicMBean distrMBean = distrSme.getMBean();
    if (distrMBean != null)
      mbs.registerMBean(distrMBean, new ObjectName("Sponsored:mbean=distribution"));

    final LoggingMBean lb = new LoggingMBean("Sponsored", LogManager.getLoggerRepository());
    mbs.registerMBean(lb, new ObjectName("Sponsored:mbean=logging"));

    // Load JMX configuration
    final int jmxPort=xmlConfig.getSection("jmx").getInt("port");
    final String jmxUser=xmlConfig.getSection("jmx").getString("user");
    final String jmxPassword=xmlConfig.getSection("jmx").getString("password");

    HtmlAdaptorServer adapter = new HtmlAdaptorServer(jmxPort, new AuthInfo[] {new AuthInfo(jmxUser, jmxPassword)});
    mbs.registerMBean(adapter, new ObjectName("Sponsored:mbean=adaptor"));
    adapter.start();
  }

  private static class ShutdownHook extends Thread {
    private final SubscriptionSme subscriptionSme;
    private final DistributionSme distributionSme;
    private final ExecutorService configReloader;

    public ShutdownHook(SubscriptionSme subscriptionSme, DistributionSme distributionSme, ExecutorService configReloader) {
      this.subscriptionSme = subscriptionSme;
      this.distributionSme = distributionSme;
      this.configReloader = configReloader;
    }

    public void run() {
      System.out.println("Shutdown called");
      try {
        subscriptionSme.stop();
      } catch (Throwable e) {
        e.printStackTrace();
      }
      try {
        distributionSme.stop();
      } catch (Throwable e) {
        e.printStackTrace();
      }
      configReloader.shutdown();
    }
  }

  protected static class ConfigReloadTask implements Runnable {
    private static final Category log = Category.getInstance("DISTRIBUTION");

    private final SmscTimezonesList timezones;
    private final String timezonesFile;
    private final String routesFile;

    public ConfigReloadTask(SmscTimezonesList timezones, String timezonesFile, String routesFile) {
      this.timezones = timezones;
      this.timezonesFile = timezonesFile;
      this.routesFile = routesFile;
    }

    public void run() {
      try {
        timezones.load(timezonesFile, routesFile);
      } catch (SmscTimezonesListException e) {
        log.error("Timezones reload failed", e);
      }

      if (log.isDebugEnabled())
        log.debug("Timezones have been reloaded.");
    }
  }

  protected static class InitException extends RuntimeException {

    public InitException(String message) {
      super(message);
    }

    public InitException(Throwable cause) {
      super(cause);
    }
  }

  private static class Multiplexor extends TestMultiplexor {

    private volatile long msgId;    

    public void sendResponse(PDU pdu) {
    }

    public void _sendMessage(Message message) throws SMPPException {
      SimpleResponse simpleResponse = new SimpleResponse(0,0,0,String.valueOf(msgId++), message);
      simpleResponse.setStatus(Data.ESME_ROK);
      simpleResponse.setStatusClass(PDU.STATUS_CLASS_NO_ERROR);
      handleResponse(simpleResponse);      
    }

    public void connect() throws SMPPException {
      super.connect();
    }

    public void shutdown() {
      super.shutdown();
    }
  }
}

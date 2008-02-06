package com.eyeline.sponsored;

import com.eyeline.sme.smppcontainer.SMPPServiceContainer;
import com.eyeline.sponsored.distribution.advert.distr.DistributionSme;
import com.eyeline.sponsored.subscription.service.SubscriptionSme;
import com.eyeline.sponsored.config.Config;
import com.eyeline.utils.config.xml.XmlConfig;
import org.apache.log4j.Category;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesList;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesListException;

import java.io.File;

/**
 * User: artem
 * Date: 01.02.2008
 */

public class Sme {

  public static void main(String[] args) {
    SMPPServiceContainer container = null;
    SubscriptionSme subscriptionSme = null;
    DistributionSme distributionSme = null;

    try {
      final XmlConfig xmlConfig = new XmlConfig(new File("conf/config.xml"));
      xmlConfig.load();

      final Config conf = new Config(xmlConfig);

      final SmscTimezonesList timezones = new SmscTimezonesList(conf.getTimezonesFile(), conf.getRoutesFile());

      container = new SMPPServiceContainer();
      container.init(conf.getContainerConfigFile(), conf.getSmppConfigFile());
      
      subscriptionSme = new SubscriptionSme(xmlConfig, timezones);
      distributionSme = new DistributionSme(xmlConfig, timezones, container.getOutgoingQueue());

      new ConfigUpdater(600000, timezones).start();

      container.start();

    } catch (Throwable e) {
      e.printStackTrace();
      container.stop();
      subscriptionSme.stop();
      distributionSme.stop();
    }
  }

  protected static class ConfigUpdater extends Thread {
    private static final Category log = Category.getInstance("DISTRIBUTION");

    private final long delay;
    private final SmscTimezonesList timezones;

    public ConfigUpdater(long delay, SmscTimezonesList timezones) {
      this.delay = delay;
      this.timezones = timezones;
    }

    public synchronized void run() {
      try {
        wait(delay);
      } catch (InterruptedException e) {
      }

      try {
        timezones.reload();
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
}

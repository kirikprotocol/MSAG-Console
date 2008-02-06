package com.eyeline.sponsored.subscription.service;

import com.eyeline.sme.test.smpp.ConsoleBasedMultiplexor;
import com.eyeline.sme.smppcontainer.SMPPServiceContainer;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.sponsored.subscription.config.Config;

import java.io.File;

import ru.sibinco.smsc.utils.timezones.SmscTimezonesList;

/**
 *
 * @author artem
 */
public class SmeConsoleTest extends SubscriptionSme {

  public SmeConsoleTest(XmlConfig config, SmscTimezonesList timezones) {
    super(config, timezones);
  }

  public static void main(String[] args) {
    final ConsoleBasedMultiplexor m = new ConsoleBasedMultiplexor("+79139023974", "741", "smsx", true);

    SMPPServiceContainer container = null;
    SubscriptionSme sme = null;
    try {
      final XmlConfig xmlConfig = new XmlConfig(new File("conf/config.xml"));
      xmlConfig.load();

      Config conf = new Config(xmlConfig);

      final SmscTimezonesList timezones = new SmscTimezonesList(conf.getTimezonesFile(), conf.getRoutesFile());
      container = new SMPPServiceContainer(m);
      container.init(conf.getContainerConfigFile(), conf.getSmppConfigFile());

      sme = new SubscriptionSme(xmlConfig, timezones);

      container.start();

    } catch (Exception e) {
      e.printStackTrace();
      container.stop();
      sme.stop();
    }
  }
}

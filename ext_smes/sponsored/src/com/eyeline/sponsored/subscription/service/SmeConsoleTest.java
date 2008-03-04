package com.eyeline.sponsored.subscription.service;

import com.eyeline.sme.handler.MessageHandler;
import com.eyeline.sme.smpp.SMPPTransceiver;
import com.eyeline.sme.smpp.test.ConsoleBasedMultiplexor;
import com.eyeline.sponsored.subscription.config.Config;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesList;

import java.io.File;

/**
 *
 * @author artem
 */
public class SmeConsoleTest extends SubscriptionSme {

  public SmeConsoleTest(XmlConfig config, SmscTimezonesList timezones) {
    super(config, timezones);
  }

  public static void main(String[] args) {

    SMPPTransceiver smppTranceiver = null;
    SubscriptionSme sme = null;
    MessageHandler handler = null;

    try {
      final XmlConfig xmlConfig = new XmlConfig(new File("conf/config.xml"));
      xmlConfig.load();

      Config conf = new Config(xmlConfig);

      final PropertiesConfig smppProps = new PropertiesConfig(conf.getSmppConfigFile());

      smppTranceiver = new SMPPTransceiver(new ConsoleBasedMultiplexor("+79139023974", "741", "smsx", true), smppProps, "");

      handler = new MessageHandler(conf.getHandlerConfigFile(), smppTranceiver.getInQueue(), smppTranceiver.getOutQueue());

      final SmscTimezonesList timezones = new SmscTimezonesList(conf.getTimezonesFile(), conf.getRoutesFile());

      sme = new SubscriptionSme(xmlConfig, timezones);

      smppTranceiver.connect();
      handler.start();

    } catch (Exception e) {
      e.printStackTrace();
      sme.stop();
    }

  }
}

package com.eyeline.sponsored.subscription.service;

import com.eyeline.sme.handler.MessageHandler;
import com.eyeline.sme.smpp.SMPPTransceiver;
import com.eyeline.sponsored.Sme;
import com.eyeline.sponsored.ds.subscription.impl.db.DBSubscriptionDataSource;
import com.eyeline.sponsored.subscription.config.Config;
import com.eyeline.sponsored.subscription.service.core.SubscriptionProcessor;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesList;

import java.io.File;

/**
 *
 * @author artem
 */
public class SubscriptionSme extends Sme{

  private final DBSubscriptionDataSource ds;

  public SubscriptionSme(XmlConfig config, SmscTimezonesList timezones) {
    try {
      
      Config c = new Config(config);

      // Init data source
      ds = new DBSubscriptionDataSource(new PropertiesConfig(c.getSubscriptionSql()));
      ds.init(c.getStorageDriver(), c.getStorageUrl(), c.getStorageLogin(), c.getStoragePwd(), c.getStorageConnTimeout(), c.getStoragePoolSize());

      // Init processor
      SubscriptionProcessor.init(ds, timezones);

    } catch (Exception e) {
      e.printStackTrace();
      throw new InitException("");
    }
  }

  public void stop() {
    ds.shutdown();
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

      smppTranceiver = new SMPPTransceiver(smppProps, "");

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

  private static class InitException extends RuntimeException {

    public InitException(String message) {
      super(message);
    }
  }
}

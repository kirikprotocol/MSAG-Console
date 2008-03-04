package com.eyeline.sponsored.distribution.advert.distr;

import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.sme.smpp.SMPPTransceiver;
import com.eyeline.sme.handler.MessageHandler;
import com.eyeline.sponsored.Sme;
import com.eyeline.sponsored.distribution.advert.config.Config;
import com.eyeline.sponsored.distribution.advert.config.DistributionInfo;
import com.eyeline.sponsored.distribution.advert.distr.adv.AdvertisingClient;
import com.eyeline.sponsored.distribution.advert.distr.core.DeliveryStatsProcessor;
import com.eyeline.sponsored.distribution.advert.distr.core.ConservativeDistributionEngine;
import com.eyeline.sponsored.distribution.advert.distr.core.DistributionEngine;
import com.eyeline.sponsored.distribution.advert.distr.core.IntervalDistributionEngine;
import com.eyeline.sponsored.ds.distribution.advert.impl.db.DBDistributionDataSource;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesList;

import java.io.File;
import java.util.Iterator;

/**
 * User: artem
 * Date: 01.02.2008
 */

public class DistributionSme extends Sme {

  private DBDistributionDataSource distrDS;
  private AdvertisingClient advClient;
  private DistributionEngine distrEngine;

  public DistributionSme(XmlConfig config, SmscTimezonesList timezones, OutgoingQueue outQueue) {

    try {
      Config c = new Config(config);

      // Init distr data source
      distrDS = new DBDistributionDataSource(new PropertiesConfig(c.getStorageDistributionSql()));
      distrDS.init(c.getStorageDriver(), c.getStorageUrl(), c.getStorageLogin(), c.getStoragePwd(), c.getStorageConnTimeout(), c.getStoragePoolSize());

      // Init delivery stats processor
      DeliveryStatsProcessor.init(distrDS, timezones);

      // Init advertising client
      advClient = new AdvertisingClient(c.getAdvertisingHost(), c.getAdvertisingPort(), c.getAdvertisingConnTimeout());
      advClient.connect();

      // Init distribution engine
      if (c.getEngineType().equals("conservative")) {
        ConservativeDistributionEngine engine = new ConservativeDistributionEngine(outQueue, distrDS, advClient);
        engine.init(c.getDeliveriesSendSpeedLimit(), c.getDeliveriesFetchInterval());
        distrEngine = engine;
      } else if (c.getEngineType().equals("interval")) {
        IntervalDistributionEngine engine = new IntervalDistributionEngine(outQueue, distrDS, advClient);
        engine.init(c.getDeliveriesFetchInterval(), c.getDeliveriesPrepareInterval(), c.getDeliveriesSendSpeedLimit());
        distrEngine = engine;
      } else
        throw new InitException("Unknown distribution engine type: " + c.getEngineType());

      // Load distribution infos
      for (DistributionInfo distributionInfo : c.getDistrInfos()) distrEngine.addDistribution(distributionInfo);

      distrEngine.start();

    } catch (Exception e) {
      throw new InitException(e);
    }
  }

  public void stop() {
    distrEngine.stop();
    advClient.close();
    distrDS.shutdown();
  }

  public static void main(String[] args) {

    SMPPTransceiver smppTranceiver = null;
    DistributionSme sme = null;
    MessageHandler handler = null;

    try {
      final XmlConfig config = new XmlConfig(new File("conf/config.xml"));
      config.load();

      Config c= new Config(config);

      final PropertiesConfig smppProps = new PropertiesConfig(c.getSmppConfigFile());

      smppTranceiver = new SMPPTransceiver(smppProps, "");

      handler = new MessageHandler(c.getHandlerConfigFile(), smppTranceiver.getInQueue(), smppTranceiver.getOutQueue());

      final SmscTimezonesList timezones = new SmscTimezonesList(c.getTimezonesFile(), c.getRoutesFile());

      sme = new DistributionSme(config, timezones, smppTranceiver.getOutQueue());

      smppTranceiver.connect();
      handler.start();

    } catch (Exception e) {
      e.printStackTrace();
      sme.stop();
    }
  }


}

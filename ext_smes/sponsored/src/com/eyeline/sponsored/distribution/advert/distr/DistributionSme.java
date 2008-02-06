package com.eyeline.sponsored.distribution.advert.distr;

import com.eyeline.sme.smppcontainer.SMPPServiceContainer;
import com.eyeline.sme.utils.smpp.OutgoingQueue;
import com.eyeline.sponsored.Sme;
import com.eyeline.sponsored.distribution.advert.config.Config;
import com.eyeline.sponsored.distribution.advert.config.DistributionInfo;
import com.eyeline.sponsored.distribution.advert.distr.adv.AdvertisingClient;
import com.eyeline.sponsored.distribution.advert.distr.core.DeliveryStatsProcessor;
import com.eyeline.sponsored.distribution.advert.distr.core.DistributionEngine;
import com.eyeline.sponsored.ds.distribution.advert.impl.db.DBDistributionDataSource;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesList;

import java.io.File;
import java.util.Iterator;
import java.util.List;

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

      // Load distribution infos
      final List<DistributionInfo> distrInfos = c.getDistrInfos();

      // Init distribution engine
      distrEngine = new DistributionEngine(outQueue, distrDS, advClient);
      distrEngine.init(c.getDeliveriesSendSpeedLimit(), c.getDeliveriesFetchInterval());
      for (Iterator<DistributionInfo> iter = c.getDistrInfos().iterator(); iter.hasNext();)
        distrEngine.addDistribution(iter.next());

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
    SMPPServiceContainer container = null;
    DistributionSme sme = null;
    try {

      final XmlConfig config = new XmlConfig(new File("conf/config.xml"));
      config.load();

      Config c= new Config(config);

      final SmscTimezonesList timezones = new SmscTimezonesList(c.getTimezonesFile(), c.getRoutesFile());

      container = new SMPPServiceContainer();
      container.init(c.getContainerConfigFile(), c.getSmppConfigFile());

      sme = new DistributionSme(config, timezones, container.getOutgoingQueue());

      container.start();

    } catch (Exception e) {
      e.printStackTrace();
      container.stop();
      sme.stop();
    }
  }


}

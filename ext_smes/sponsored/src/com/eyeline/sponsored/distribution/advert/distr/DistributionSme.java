package com.eyeline.sponsored.distribution.advert.distr;

import com.eyeline.sme.handler.MessageHandler;
import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.sme.smpp.SMPPTransceiver;
import com.eyeline.sponsored.Sme;
import com.eyeline.sponsored.distribution.advert.config.Config;
import com.eyeline.sponsored.distribution.advert.config.DistributionInfo;
import com.eyeline.sponsored.distribution.advert.deliveries.DeliveriesGenerator;
import com.eyeline.sponsored.distribution.advert.distr.adv.AdvertisingClientFactory;
import com.eyeline.sponsored.distribution.advert.distr.core.ConservativeDistributionEngine;
import com.eyeline.sponsored.distribution.advert.distr.core.DeliveryStatsProcessor;
import com.eyeline.sponsored.distribution.advert.distr.core.DistributionEngine;
import com.eyeline.sponsored.distribution.advert.distr.core.IntervalDistributionEngine;
import com.eyeline.sponsored.ds.banner.BannerMap;
import com.eyeline.sponsored.ds.banner.BannerMapMBean;
import com.eyeline.sponsored.ds.banner.impl.JNIBannerMapImpl;
import com.eyeline.sponsored.ds.distribution.advert.DeliveriesDataSource;
import com.eyeline.sponsored.ds.distribution.advert.DeliveryStatsDataSource;
import com.eyeline.sponsored.ds.distribution.advert.impl.db.DBDistributionDataSource;
import com.eyeline.sponsored.ds.distribution.advert.impl.file.deliveries.FileDeliveriesDataSource;
import com.eyeline.sponsored.ds.distribution.advert.impl.file.deliverystats.FileDeliveryStatDataSource;
import com.eyeline.sponsored.ds.subscription.impl.db.DBSubscriptionDataSource;
import com.eyeline.sponsored.utils.CalendarUtils;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import ru.sibinco.smsc.utils.timezones.SmscTimezone;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesList;

import java.io.File;
import java.util.Date;
import java.util.Iterator;
import java.util.TimeZone;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;

/**
 * User: artem
 * Date: 01.02.2008
 */

public class DistributionSme extends Sme {

  private DeliveriesDataSource deliveriesDataSource;
  private DeliveryStatsDataSource deliveryStatsDataSource;
  private DeliveriesGenerator deliveriesGenerator;
  private DBSubscriptionDataSource subscriptionDataSource;
  private ScheduledExecutorService deliveriesGeneratorExecutor;
  private DistributionEngine distrEngine;
  private BannerMap bannerMap;

  public DistributionSme(XmlConfig config, SmscTimezonesList timezones, OutgoingQueue outQueue) {

    try {
      Config c = new Config(config);

      // Init distr data source
      if (c.getDeliveriesDataSource().equals("db")) {
        deliveriesDataSource = new DBDistributionDataSource(new PropertiesConfig(c.getStorageDistributionSql()));
        ((DBDistributionDataSource)deliveriesDataSource).init(c.getStorageDriver(), c.getStorageUrl(), c.getStorageLogin(), c.getStoragePwd(), c.getStorageConnTimeout(), c.getStoragePoolSize());
      } else if (c.getDeliveriesDataSource().equals("file")) {
        deliveriesDataSource = new FileDeliveriesDataSource(c.getFileStorageStoreDir());
      } else
        throw new InitException("Unknown deliveries storage type: " + c.getDeliveriesDataSource());

      // Init delivery stats processor
      if (c.getDeliveryStatsDataSource().equals("db")) {
        if (c.getDeliveriesDataSource().equals("db"))
          deliveryStatsDataSource = (DBDistributionDataSource)deliveriesDataSource;
        else {
          deliveryStatsDataSource = new DBDistributionDataSource(new PropertiesConfig(c.getStorageDistributionSql()));
          ((DBDistributionDataSource)deliveryStatsDataSource).init(c.getStorageDriver(), c.getStorageUrl(), c.getStorageLogin(), c.getStoragePwd(), c.getStorageConnTimeout(), c.getStoragePoolSize());
        }
      } else if (c.getDeliveryStatsDataSource().equals("file")) {
        deliveryStatsDataSource = new FileDeliveryStatDataSource(c.getFileStorageStoreDir());
      } else
        throw new InitException("Unknown delivery stats storage type: " + c.getDeliveryStatsDataSource());

      // Init subscription data source
      subscriptionDataSource = new DBSubscriptionDataSource(new PropertiesConfig(c.getStorageSubscriptionSql()));
      subscriptionDataSource.init(c.getStorageDriver(), c.getStorageUrl(), c.getStorageLogin(), c.getStoragePwd(), c.getStorageConnTimeout(), c.getStoragePoolSize());

      // Init advertising client factory
      AdvertisingClientFactory advertisingClientFactory = new AdvertisingClientFactory(c.getAdvertisingHost(), c.getAdvertisingPort(), c.getAdvertisingConnTimeout());

      // Init banner map
      bannerMap = new JNIBannerMapImpl(new File(c.getFileStorageStoreDir(), "banner.bin").getAbsolutePath(), 500, 20000,
                                      new File(c.getFileStorageStoreDir(), "banner_keys.bin").getAbsolutePath());

      DeliveryStatsProcessor.init(deliveryStatsDataSource, timezones, bannerMap);

      // Init distribution engine
      if (c.getEngineType().equals("conservative")) {
        ConservativeDistributionEngine engine = new ConservativeDistributionEngine(outQueue,
                                                                                   deliveriesDataSource,
                                                                                   advertisingClientFactory,
                                                                                   bannerMap);
        engine.init(c.getDeliveriesSendSpeedLimit(), c.getDeliveriesFetchInterval());
        distrEngine = engine;
      } else if (c.getEngineType().equals("interval")) {
        distrEngine = new IntervalDistributionEngine(outQueue,
                                                     deliveriesDataSource,
                                                     advertisingClientFactory,
                                                     bannerMap,
                                                     c.getDeliveriesFetchInterval(),
                                                     c.getDeliveriesPrepareInterval(),
                                                     c.getPoolSize());        
      } else
        throw new InitException("Unknown distribution engine type: " + c.getEngineType());

      // Init deliveries generator
      deliveriesGenerator = new DeliveriesGenerator(deliveriesDataSource, subscriptionDataSource);
      for (Iterator iter = timezones.getTimezones().iterator(); iter.hasNext();) {
        SmscTimezone tz = (SmscTimezone)iter.next();
        deliveriesGenerator.addTimezone(TimeZone.getTimeZone(tz.getName()));
      }

      // Load distribution infos
      for (DistributionInfo distributionInfo : c.getDistrInfos()) {
        distrEngine.addDistribution(distributionInfo);
        deliveriesGenerator.addDistribution(distributionInfo);
      }

      distrEngine.start();

      deliveriesGeneratorExecutor = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
        public Thread newThread(Runnable r) {
          return new Thread(r, "delivGen");
        }
      });

      deliveriesGeneratorExecutor.scheduleAtFixedRate(new Runnable() {
        public void run() {
          deliveriesGenerator.run();
        }
      }, CalendarUtils.getNextHourStart(new Date()).getTime() - System.currentTimeMillis(), 3600000, TimeUnit.MILLISECONDS);

    } catch (Exception e) {
      throw new InitException(e);
    }
  }

  public BannerMapMBean getBannerMapMBean() {
    return new BannerMapMBean(bannerMap);
  }

  public void stop() {
    distrEngine.stop();
    deliveriesDataSource.shutdown();
    deliveryStatsDataSource.shutdown();
    subscriptionDataSource.shutdown();
    deliveriesGeneratorExecutor.shutdown();
    bannerMap.close();
  }

  public static void main(String[] args) {

    SMPPTransceiver smppTranceiver = null;
    DistributionSme sme = null;
    MessageHandler handler = null;

    try {
      final XmlConfig config = new XmlConfig();
      config.load(new File("conf/config.xml"));

      Config c= new Config(config);

      final PropertiesConfig smppProps = new PropertiesConfig(c.getSmppConfigFile());

      smppTranceiver = new SMPPTransceiver(smppProps, "");

      handler = new MessageHandler(c.getHandlerConfigFile(), smppTranceiver.getInQueue(), smppTranceiver.getOutQueue());

      final SmscTimezonesList timezones = new SmscTimezonesList();
      timezones.load(c.getTimezonesFile(), c.getRoutesFile());

      sme = new DistributionSme(config, timezones, smppTranceiver.getOutQueue());

      smppTranceiver.connect();
      handler.start();

    } catch (Exception e) {
      e.printStackTrace();
      sme.stop();
    }
  }


}

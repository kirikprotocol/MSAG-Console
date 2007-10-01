package ru.sibinco.smsx.engine.service.sponsored;

import com.eyeline.sme.utils.config.properties.PropertiesConfig;
import ru.sibinco.smsx.engine.service.ServiceInitializationException;
import ru.sibinco.smsx.engine.service.sponsored.datasource.DBSponsoredDataSource;
import ru.sibinco.smsx.engine.service.sponsored.datasource.SponsoredDataSource;
import ru.sibinco.smsx.engine.service.sponsored.commands.*;
import ru.sibinco.smsx.network.smppnetwork.SMPPOutgoingQueue;

import java.io.File;
import java.text.SimpleDateFormat;

/**
 * User: artem
 * Date: 02.07.2007
 */

class SponsoredServiceImpl implements SponsoredService {

  private final SponsoredEngine engine;
  private final ConfigChecker checker;
  private final SponsoredProcessor processor;
  private final SponsoredDataSource ds;

  SponsoredServiceImpl(String configDir, SMPPOutgoingQueue outQueue) {
    try {
      final PropertiesConfig serviceConfig = new PropertiesConfig(new File(configDir, "services/sponsored/service.properties"));

      ds = new DBSponsoredDataSource(new File(configDir, "services/sponsored/sponsored.sql").getAbsolutePath(), "");

      // Read allowed counts
      final int[] allowedCountsArray = serviceConfig.getIntList("allowed.counts", ",");

      engine = new SponsoredEngine(outQueue, ds, allowedCountsArray);
      engine.setAdvertisingClientName(serviceConfig.getString("advertising.service.name"));
      engine.setFromHour(serviceConfig.getInt("from.hour"));
      engine.setToHour(serviceConfig.getInt("to.hour"));
      engine.setSmeAddress(serviceConfig.getString("service.address"));

      final SimpleDateFormat df = new SimpleDateFormat("dd.MM.yyyy");

      processor = new SponsoredProcessor(engine, ds,
                                         serviceConfig.getInt("from.hour"),
                                         serviceConfig.getInt("to.hour"),
                                         df.parse(serviceConfig.getString("subscription.start.date")),
                                         serviceConfig.getInt("subscription.interval"),
                                         allowedCountsArray);

      if (!serviceConfig.getBool("subscription"))
        processor.lockSubscription();

      checker = new ConfigChecker(new File(configDir, "services/sponsored/service.properties").getAbsolutePath(), 60000, processor);

    } catch (Throwable e) {
      throw new ServiceInitializationException(e);
    }
  }

  public void execute(SponsoredRegisterAbonentCmd cmd) {
    processor.execute(cmd);
  }

  public void execute(SponsoredUnregisterAbonentCmd cmd) {
    processor.execute(cmd);
  }

  public void execute(SponsoredRegisterDeliveryCmd cmd) {
    processor.execute(cmd);
  }

  public void startService() {
    engine.startService();
    checker.start();
  }

  public void stopService() {
    engine.stopService();
    checker.stop();
    ds.release();
  }

}

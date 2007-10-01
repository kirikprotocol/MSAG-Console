package ru.sibinco.smsx.engine.service.sponsored;

import com.eyeline.sme.utils.worker.IterativeWorker;
import com.eyeline.sme.utils.config.properties.PropertiesConfig;
import com.eyeline.sme.utils.config.ConfigException;
import org.apache.log4j.Category;

/**
 * User: artem
 * Date: 16.07.2007
 */

class ConfigChecker extends IterativeWorker {

  private static final Category log = Category.getInstance("SPONSORED");


  private final String configFile;
  private final SponsoredProcessor processor;

  ConfigChecker(String configFile, int checkInterval, SponsoredProcessor processor) {
    super(log, checkInterval);

    this.configFile = configFile;
    this.processor = processor;
  }

  public void iterativeWork() {
    try {
      final PropertiesConfig cfg = new PropertiesConfig(configFile);

      if (!cfg.getBool("subscription"))
        processor.lockSubscription();
      else
        processor.unlockSubscription();

    } catch (ConfigException e) {
      log.error(e,e);
    }
  }

  protected void stopCurrentWork() {
  }
}

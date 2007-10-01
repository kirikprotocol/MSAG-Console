package ru.sibinco.smsx;

import com.eyeline.sme.utils.config.properties.PropertiesConfig;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.smsx.engine.SmeEngine;
import ru.sibinco.smsx.network.advertising.AdvertisingClientFactory;
import ru.sibinco.smsx.network.dbconnection.ConnectionPoolFactory;
import ru.sibinco.smsx.network.smppnetwork.SMPPMultiplexor;
import ru.sibinco.smsx.network.personalization.PersonalizationClientPoolFactory;

import java.io.File;

import org.apache.log4j.Category;

/**
 * User: artem
 * Date: 03.07.2007
 */

public class Sme {

  private static final Category log = Category.getInstance("SME");

  private SMPPMultiplexor smppMultiplexor;
  private SmeEngine smeEngine;

  public Sme(String configDir) throws SmeException {

    smppMultiplexor = null;
    smeEngine = null;

    try {
      // Init DB connection pool
      ConnectionPoolFactory.init(configDir);

      // Init personalization clients factory
      PersonalizationClientPoolFactory.init(configDir);

      // Init advertising clients factory
      AdvertisingClientFactory.init(configDir);

      // Init SMPP multiplexor
      smppMultiplexor = new SMPPMultiplexor(new PropertiesConfig(new File(configDir, "smpp.properties")));
      smppMultiplexor.connect();

      // Init and start SME engine
      smeEngine = new SmeEngine(configDir, smppMultiplexor);

    } catch (Throwable e) {
      log.error(e,e);
      throw new SmeException(e);
    }
  }

  public void release() throws SmeException {
    log.error("Stopping smpp multiplexor...");
    try {
      smppMultiplexor.shutdown();
    } catch (SMPPException e) {
      throw new SmeException(e);
    }

    log.error("Stopping sme engine...");
    smeEngine.release();
  }
}

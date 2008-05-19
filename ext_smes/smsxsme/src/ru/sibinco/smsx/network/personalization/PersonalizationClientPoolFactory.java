package ru.sibinco.smsx.network.personalization;

import com.eyelinecom.whoisd.personalization.PersonalizationClientPool;
import com.eyelinecom.whoisd.personalization.exceptions.PersonalizationClientException;
import com.eyeline.utils.config.properties.PropertiesConfig;
import ru.sibinco.smsx.InitializationException;

import java.io.File;

/**
 * User: artem
 * Date: 10.07.2007
 */

/**
 * Factory for personalization client
 */
public class PersonalizationClientPoolFactory {

  private static PropertiesConfig cfg;

  /**
   * Initiate factory from "personalization.properties" file
   * @param configDir directory contains "personalization.properties" file
   */
  public static void init(String configDir) {
    try {
      cfg = new PropertiesConfig(new File(configDir, "personalization.properties").getAbsolutePath());
    } catch (Throwable e) {
      throw new InitializationException(e);
    }
  }

  /**
   * Create new instance of personalization client
   * @return new instance of personalization client
   * @throws PersonalizationClientException
   */
  public static PersonalizationClientPool createPool() throws PersonalizationClientException {
    return new PersonalizationClientPool(cfg);
  }



}

package ru.sibinco.smsx.network.personalization;

import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyelinecom.whoisd.personalization.PersonalizationClientPool;
import com.eyelinecom.whoisd.personalization.exceptions.PersonalizationClientException;
import ru.sibinco.smsx.InitializationException;

import java.util.concurrent.CountDownLatch;

/**
 * User: artem
 * Date: 10.07.2007
 */

/**
 * Factory for personalization client
 */
public class PersonalizationClientPoolFactory {

  private static PropertiesConfig cfg;
  private static CountDownLatch initLatch = new CountDownLatch(1);

  /**
   * Initiate factory from "personalization.properties" file
   * @param c XmlConfig
   */
  public static void init(XmlConfig c) {
    try {
      cfg = new PropertiesConfig(c.getSection("personalization").toProperties("personalization.", "."));
      initLatch.countDown();
    } catch (Throwable e) {
      throw new InitializationException(e);
    }
  }

  /**
   * Create new instance of personalization client
   * @return new instance of personalization client
   * @throws PersonalizationClientException if can't create personalization client pool
   */
  public static PersonalizationClientPool createPool() throws PersonalizationClientException {
    try {
      initLatch.await();
    } catch (InterruptedException e) {
      return null;
    }
    return new PersonalizationClientPool(cfg);
  }



}

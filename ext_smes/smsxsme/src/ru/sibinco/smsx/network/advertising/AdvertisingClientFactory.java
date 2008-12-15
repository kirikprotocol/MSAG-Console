package ru.sibinco.smsx.network.advertising;

import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.xml.XmlConfigSection;
import ru.sibinco.smsx.InitializationException;

import java.util.concurrent.CountDownLatch;

/**
 * User: artem
 * Date: 16.07.2007
 */

/**
 * This is a factory for AdvertisingClient.
 */
public class AdvertisingClientFactory {

  private static String host;
  private static int port;
  private static long timeout;
  private static final CountDownLatch initLatch = new CountDownLatch(1);

  /**
   * Init factory from file "advertising.properties"
   * @param c XmlConfig
   */
  public static void init(XmlConfig c) {
    try {
      XmlConfigSection section = c.getSection("advertising");

      host = section.getString("host");
      port = section.getInt("port");
      timeout = section.getLong("timeout");

      initLatch.countDown();
    } catch (ConfigException e) {
      throw new InitializationException(e.getMessage());
    }
  }

  /**
   * Create new instance of AdvertisingClient
   * @return new instance of AdvertisingClient
   */
  public static AdvertisingClient createAdvertisingClient() {
    try {
      initLatch.await();
    } catch (InterruptedException e) {
      return null;
    }
    return new AdvertisingClientImpl(host, port, timeout);
  }

}

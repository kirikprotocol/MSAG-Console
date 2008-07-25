package ru.sibinco.smsx.network.advertising;

import ru.sibinco.smsx.InitializationException;

import java.io.File;

import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.config.xml.XmlConfig;

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

  /**
   * Init factory from file "advertising.properties"
   * @param configDir directory contains file "advertising.properties"
   */
  public static void init(String configDir) {
    try {
      final XmlConfig c = new XmlConfig();
      c.load(new File(configDir, "config.xml"));
      final PropertiesConfig config = new PropertiesConfig(c.getSection("advertising").toProperties("."));            

      host = config.getString("host");
      port = config.getInt("port");
      timeout = config.getLong("timeout");

    } catch (ConfigException e) {
      throw new InitializationException("Invalid config file " + new File(configDir, "advertising.properties").getAbsolutePath() + ": " + e);
    }
  }

  /**
   * Create new instance of AdvertisingClient
   * @return new instance of AdvertisingClient
   */
  public static AdvertisingClient createAdvertisingClient() {
    return new AdvertisingClientImpl(host, port, timeout);
  }

}

package ru.sibinco.smsx.network.advertising;

import ru.sibinco.smsx.InitializationException;

import java.io.File;
import java.util.Collection;
import java.util.LinkedList;
import java.util.Iterator;

import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.ConfigException;

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
      final PropertiesConfig config = new PropertiesConfig(new File(configDir, "advertising.properties").getAbsolutePath());

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

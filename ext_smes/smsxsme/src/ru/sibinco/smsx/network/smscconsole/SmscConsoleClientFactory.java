package ru.sibinco.smsx.network.smscconsole;


import java.io.File;

import ru.sibinco.smsx.InitializationException;
import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.properties.PropertiesConfig;

/**
 * User: artem
 * Date: 17.07.2007
 */

/**
 * Factory for SmscConsoleClient
 */
public class SmscConsoleClientFactory {
  private static String host;
  private static int port;
  private static String login;
  private static String password;
  private static int timeout;

  /**
   * Initiate factory using "smscconsole.properties" file
   * @param configDir directory contains "smscconsole.properties" file
   */
  public static void init(String configDir) {
    try {
      final XmlConfig c = new XmlConfig();
      c.load(new File(configDir, "config.xml"));
      final PropertiesConfig config = new PropertiesConfig(c.getSection("smscconsole").toProperties("."));
      
      host = config.getString("host");
      port = config.getInt("port");
      login = config.getString("login");
      password = config.getString("password");
      timeout = config.getInt("timeout");
    } catch (ConfigException e) {
      throw new InitializationException(e);
    }
  }

  private static SmscConsoleClient initClient() {
    return new SmscConsoleClientImpl(host, port, timeout, login, password);
  }

  public static SmscConsoleClient getSmscConsoleClient() {
    return initClient();
  }
}

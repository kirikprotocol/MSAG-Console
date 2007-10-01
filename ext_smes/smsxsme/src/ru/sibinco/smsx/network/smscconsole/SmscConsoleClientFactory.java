package ru.sibinco.smsx.network.smscconsole;

import com.eyeline.sme.utils.config.properties.PropertiesConfig;
import com.eyeline.sme.utils.config.ConfigException;

import java.io.File;

import ru.sibinco.smsx.InitializationException;

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
      final PropertiesConfig config = new PropertiesConfig(new File(configDir, "smscconsole.properties").getAbsolutePath());
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

  /**
   * Create new instance of SmscConsoleClient
   * @return
   */
  public static SmscConsoleClient getSmscConsoleClient() {
    return initClient();
  }
}

package ru.sibinco.otasme.utils;

import snaq.db.ConnectionPool;
import com.eyeline.sme.utils.config.properties.PropertiesConfig;
import com.eyeline.sme.utils.config.ConfigException;

import java.io.File;
import java.sql.DriverManager;
import java.sql.Driver;

import ru.sibinco.otasme.InitializationException;

/**
 * User: artem
 * Date: Oct 5, 2007
 */

public class ConnectionPoolFactory {

  private static String url;
  private static String login;
  private static String password;

  public static void init(final String configDir) {
    try {
      final PropertiesConfig config = new PropertiesConfig(new File(configDir, "executor.pool.properties").getAbsolutePath());

      // Read main parameters
      url = config.getString("jdbc.source");
      System.out.println(url);
      login = config.getString("jdbc.user");
      password = config.getString("jdbc.password");

      // Register JDBC driver
      try {
        DriverManager.registerDriver((Driver)Class.forName(config.getString("jdbc.driver")).newInstance());
      } catch (Exception e) {
        throw new InitializationException("Can't register JDBC driver", e);
      }

    } catch (ConfigException e) {
      throw new InitializationException("Invalid config file " + new File(configDir, "executor.pool.properties").getAbsolutePath() + ": " + e.getMessage());
    }
  }

  public static ConnectionPool createConnectionPool(final String name, final int size, final long connectionTimeout) {
    final ConnectionPool pool = new ConnectionPool(name, size, size, connectionTimeout, url, login, password);
    pool.setCaching(false, true, false);
    return pool;
  }
}


package ru.sibinco.smsx.network.dbconnection;

import snaq.db.ConnectionPool;

import java.io.File;
import java.sql.DriverManager;
import java.sql.Driver;

import ru.sibinco.smsx.InitializationException;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.ConfigException;

/**
 * User: artem
 * Date: Aug 30, 2007
 */

public class ConnectionPoolFactory {

  private static String url;
  private static String login;
  private static String password;

  public static void init(final String configDir) {
    try {
      final PropertiesConfig config = new PropertiesConfig(new File(configDir, "dbpool.properties").getAbsolutePath());

      // Read main parameters
      url = config.getString("jdbc.source");
      login = config.getString("jdbc.user");
      password = config.getString("jdbc.password");

      // Register JDBC driver
      try {
        DriverManager.registerDriver((Driver)Class.forName(config.getString("jdbc.driver")).newInstance());
      } catch (Exception e) {
        throw new InitializationException("Can't register JDBC driver", e);
      }

    } catch (ConfigException e) {
      throw new InitializationException("Invalid config file " + new File(configDir, "dbpool.properties").getAbsolutePath() + ": " + e.getMessage());
    }
  }

  public static ConnectionPool createConnectionPool(final String name, final int size, final long connectionTimeout) {
    final ConnectionPool pool = new ConnectionPool(name, size, size, connectionTimeout, url, login, password);
    pool.setCaching(false, true, false);
    return pool;
  }
}

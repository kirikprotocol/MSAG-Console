package ru.sibinco.smsx.network.dbconnection;

import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.xml.XmlConfigSection;
import ru.sibinco.smsx.InitializationException;

import java.sql.Driver;
import java.sql.DriverManager;
import java.util.concurrent.CountDownLatch;

/**
 * User: artem
 * Date: Aug 30, 2007
 */

public class ConnectionPoolFactory {

  private static String url;
  private static String login;
  private static String password;
  private static final CountDownLatch initLatch = new CountDownLatch(1);

  public static void init(XmlConfig c) {
    try {
      XmlConfigSection s = c.getSection("dbpool").getSection("jdbc");

      // Read main parameters
      url = s.getString("source");
      login = s.getString("user");
      password = s.getString("password");

      // Register JDBC driver
      try {
        DriverManager.registerDriver((Driver)Class.forName(s.getString("driver")).newInstance());
      } catch (Exception e) {
        throw new InitializationException("Can't register JDBC driver", e);
      }

      initLatch.countDown();

    } catch (ConfigException e) {
      throw new InitializationException(e.getMessage());
    }
  }

  public static ConnectionPool createConnectionPool(final String name, final int size, final long connectionTimeout) {
    try {
      initLatch.await();
    } catch (InterruptedException e) {
      return null;
    }
    final snaq.db.ConnectionPool pool = new snaq.db.ConnectionPool(name, size, size, connectionTimeout, url, login, password);
    pool.setCaching(false, true, false);
    return new ConnectionPool(pool);
  }
}

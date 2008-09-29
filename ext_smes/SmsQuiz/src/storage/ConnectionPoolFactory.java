package storage;

import snaq.db.ConnectionPool;

import java.io.File;
import java.sql.DriverManager;
import java.sql.Driver;

import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.config.xml.XmlConfig;



public class ConnectionPoolFactory {

  private static String url;
  private static String login;
  private static String password;

  public static void init(final String configFile) throws StorageException {
    try {
      final XmlConfig c = new XmlConfig();
      c.load(new File(configFile));
      final PropertiesConfig config = new PropertiesConfig(c.getSection("dbpool").toProperties("."));
      url = config.getString("jdbc.source");
      login = config.getString("jdbc.user");
      password = config.getString("jdbc.password");

      // Register JDBC driver
      try {
        DriverManager.registerDriver((Driver)Class.forName(config.getString("jdbc.driver")).newInstance());
      } catch (Exception e) {
        throw new StorageException("Can't register JDBC driver", StorageException.ErrorCode.ERROR_INIT);
      }

    } catch (ConfigException e) {
        e.printStackTrace();
    }
  }

  public static ConnectionPool createConnectionPool(final String name, final int size, final long connectionTimeout) {
    final ConnectionPool pool = new ConnectionPool(name, size, size, connectionTimeout, url, login, password);
    pool.setCaching(false, true, false);
    return pool;
  }
}

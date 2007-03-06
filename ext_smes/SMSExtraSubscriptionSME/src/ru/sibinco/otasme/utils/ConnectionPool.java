package ru.sibinco.otasme.utils;

import ru.novosoft.jdbc.pool.ConnectionPoolDataSourceImpl;
import ru.novosoft.jdbc.pool.ConnectionPoolWithQueue;
import ru.novosoft.jdbc.pool.driver.DriverDataSource;
import ru.sibinco.otasme.InitializationException;

import javax.sql.DataSource;
import java.io.IOException;
import java.sql.Connection;
import java.sql.Driver;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.Properties;

public final class ConnectionPool {

  private static final org.apache.log4j.Category CAT = org.apache.log4j.Category.getInstance(ConnectionPool.class);

  private static final DataSource pooledDataSource;

  static {
    final Properties config = new Properties();
    try {
      config.load(ClassLoader.getSystemResourceAsStream("executor.pool.properties"));
    } catch (IOException e) {
      CAT.error("Error " + e.getMessage(), e);
      throw new InitializationException(e.getMessage());
    }

    try {
      DriverManager.registerDriver(((Driver) Class.forName(config.getProperty("jdbc.driver")).newInstance()));
    } catch (Exception e) {
      CAT.error("Error " + e.getMessage(), e);
      throw new InitializationException(e);
    }
    pooledDataSource = createDataSource(config);
  }

  private static DataSource createDataSource(final Properties config) throws InitializationException {
    DataSource ds = new DriverDataSource(Utils.loadString(config, "jdbc.source"),
                                         Utils.loadString(config, "jdbc.user"),
                                         Utils.loadString(config, "jdbc.password"));

    ConnectionPoolDataSourceImpl connectionPoolDataSource = new ConnectionPoolDataSourceImpl(ds);
    return createDataSource(config, connectionPoolDataSource);
  }

  private static DataSource createDataSource(final Properties config, ConnectionPoolDataSourceImpl connectionPoolDataSource) throws InitializationException {
    int size = Integer.parseInt(config.getProperty("Pool.Size", "20"));
    int timeout = Integer.parseInt(config.getProperty("Pool.getConnection.timeout", "30"));

    boolean checkOnGet = Boolean.getBoolean(config.getProperty("Pool.getConnection.check", "true"));
    try {
      ConnectionPoolWithQueue cpwq = new ConnectionPoolWithQueue(connectionPoolDataSource, size);
      cpwq.setLoginTimeout(timeout); // timeout to getConnection
      float fillFactor = Float.parseFloat(config.getProperty("Pool.fill.factor"));
      if (fillFactor < 0f || fillFactor > 1f) {
        throw new IllegalArgumentException("Illegal value for Pool.fill.factor. Must be in [0..1]. Value = " + fillFactor);
      }
      cpwq.setFillFactor(fillFactor);
      cpwq.fillConnectionPool(fillFactor);

      final String dbtype = Utils.loadString(config, "database.type");
      cpwq.setCheckMetadataOnRefresh("interbase".equals(dbtype) || "mssql".equals(dbtype));
      cpwq.setCheckMetadataOnConnectionClose("mssql".equals(dbtype));
      cpwq.setCheckConnectionOnGet(checkOnGet);

      return cpwq;
    } catch (Exception exc) {
      CAT.error("Could not create DataSource.", exc);
      throw new InitializationException(exc);
    }
  }

  public static Connection getConnection() throws SQLException {
    if (pooledDataSource == null)
      throw new SQLException("Conenction pool is not initialized");

    for (int i=0; i<2; i++) {
      try {

        Connection connection = pooledDataSource.getConnection();
        connection.setAutoCommit(true);
        return connection;

      } catch (SQLException e) {
      }
    }
    
    throw new SQLException("Can't get new connection");
  }
}

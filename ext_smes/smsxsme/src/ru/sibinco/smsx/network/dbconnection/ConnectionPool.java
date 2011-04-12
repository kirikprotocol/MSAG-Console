package ru.sibinco.smsx.network.dbconnection;

import snaq.db.CacheConnection;

import java.sql.Connection;
import java.sql.SQLException;

/**
 * User: artem
 * Date: 12.04.11
 */
public class ConnectionPool {

  private snaq.db.ConnectionPool wrapped;

  public ConnectionPool(snaq.db.ConnectionPool wrapped) {
    this.wrapped = wrapped;
  }

  public Connection getConnection() throws SQLException {
    return wrapped.getConnection();
  }

  public void invalidateConnection(Connection conn) {
    if (conn != null) {
      try {
        ((CacheConnection)conn).release();
      } catch (Exception ignored) {}
    }
  }

  public void init(int count) {
    wrapped.init(count);
  }

  public void release() {
    wrapped.release();
  }
}

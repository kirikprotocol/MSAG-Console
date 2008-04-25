package com.eyeline.sponsored.ds;

import snaq.db.ConnectionPool;

import java.sql.Connection;
import java.sql.Driver;
import java.sql.DriverManager;
import java.sql.SQLException;

/**
 * User: artem
 * Date: 01.02.2008
 */

public abstract class AbstractDBDataSource {

  private ConnectionPool pool;  

  public void init(String driver, String url, String login, String pwd, int connectionTimeout, int poolSize) throws DataSourceException {
    try {
      DriverManager.registerDriver((Driver) Class.forName(driver).newInstance());
    } catch (Exception e) {
      throw new DataSourceException("Can't register JDBC driver", e);
    }

    pool = new ConnectionPool("pool", poolSize, poolSize, connectionTimeout, url, login, pwd);
    pool.init(5);
    pool.setCaching(false, true, false);
  }

  protected Connection getConnection() throws SQLException {
    return pool.getConnection();
  }

  public void shutdown() {
    pool.close();
    pool.release();
  }

}

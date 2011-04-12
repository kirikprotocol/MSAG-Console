package ru.sibinco.smsx.engine.service.blacklist.datasource;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

import ru.sibinco.smsx.network.dbconnection.ConnectionPool;
import ru.sibinco.smsx.network.dbconnection.ConnectionPoolFactory;
import ru.sibinco.smsx.utils.DBDataSource;
import ru.sibinco.smsx.utils.DataSourceException;

/**
 * User: artem
 * Date: 10.07.2007
 */

public class DBBlackListDataSource extends DBDataSource implements BlackListDataSource {

  private final ConnectionPool pool;

  public DBBlackListDataSource() throws DataSourceException {
    super(DBBlackListDataSource.class.getResourceAsStream("blacklist.properties"), "");

    pool = ConnectionPoolFactory.createConnectionPool("blackList", Integer.MAX_VALUE, 10000);
    pool.init(1);
  }

  public void addMsisdnToBlackList(String msisdn) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("black.list.add.msisdn"));
      ps.setString(1, msisdn);

      ps.executeUpdate();
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }

  }

  public void removeMsisdnFromBlackList(String msisdn) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("black.list.remove.msisdn"));
      ps.setString(1, msisdn);

      ps.executeUpdate();
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }
  }

  public boolean isMsisdnInBlackList(String msisdn) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs;

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("black.list.find.msisdn"));
      ps.setString(1, msisdn);

      rs = ps.executeQuery();

      return rs.next();

    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }

  }

  public void release() {
    pool.release();
  }
}

package ru.sibinco.smsx.engine.service.group.datasource.impl.send;

import ru.sibinco.smsx.engine.service.group.datasource.GroupSendDataSource;
import ru.sibinco.smsx.network.dbconnection.ConnectionPoolFactory;
import ru.sibinco.smsx.utils.DBDataSource;
import ru.sibinco.smsx.utils.DataSourceException;
import snaq.db.ConnectionPool;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * User: artem
 * Date: 12.03.2009
 */
public class DBGroupSendDataSource extends DBDataSource implements GroupSendDataSource {

  private AtomicInteger id;
  private final ConnectionPool pool;

  public DBGroupSendDataSource() throws DataSourceException {
    super(DBGroupSendDataSource.class.getResourceAsStream("groupsend.properties"), "");

    pool = ConnectionPoolFactory.createConnectionPool("groupsend", Integer.MAX_VALUE, 60000);
    pool.init(1);

    id = new AtomicInteger(loadId());
  }

  private int loadId() throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("load.max.id"));

      rs = ps.executeQuery();

      return (rs.next()) ? rs.getInt(1) : 0;

    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      _close(rs, ps, conn);
    }
  }

  public int insert(Collection<String> addresses) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();
      if (conn.getAutoCommit())
        conn.setAutoCommit(false);

      int idValue = id.incrementAndGet();

      for (String address : addresses) {
        ps = conn.prepareStatement(getSql("insert"));
        ps.setInt(1, idValue);
        ps.setString(2, address);
        ps.executeUpdate();
      }

      conn.commit();
      conn.setAutoCommit(true);

      return idValue;
    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }
  }

  public Map<String, Integer> statuses(int id) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("statuses"));
      ps.setInt(1, id);
      rs = ps.executeQuery();

      if (rs != null) {
        Map<String, Integer> result = new HashMap<String, Integer>(10);
        while (rs.next())
          result.put(rs.getString(1), rs.getInt(2));
        return result;
      }

      return null;

    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      _close(rs, ps, conn);
    }
  }

  public int updateStatus(long smppId, int status) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = pool.getConnection();

      ps = conn.prepareStatement(getSql("update.status"));

      ps.setInt(1, status);
      ps.setLong(2, smppId);

      return ps.executeUpdate();

    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }
  }

  public int updateStatus(int id, String address, int status) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = pool.getConnection();

      ps = conn.prepareStatement(getSql("update.status.by.id"));

      ps.setInt(1, status);
      ps.setInt(2, id);
      ps.setString(3, address);

      return ps.executeUpdate();

    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }
  }

  public int updateSmppId(int id, String address, long smppId) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = pool.getConnection();

      ps = conn.prepareStatement(getSql("update.smpp.id"));

      ps.setLong(1, smppId);
      ps.setInt(2, id);
      ps.setString(3, address);

      return ps.executeUpdate();

    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }
  }

  public void release() {
    pool.release();
  }
}

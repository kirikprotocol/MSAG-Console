package ru.sibinco.smsx.engine.service.sender.datasource;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import ru.sibinco.smsx.network.dbconnection.ConnectionPool;
import ru.sibinco.smsx.network.dbconnection.ConnectionPoolFactory;
import ru.sibinco.smsx.utils.DBDataSource;
import ru.sibinco.smsx.utils.DataSourceException;

/**
 * User: artem
 * Date: 06.07.2007
 */

public class DBSenderDataSource extends DBDataSource implements SenderDataSource {

  private Lock idLock = new ReentrantLock();
  private AtomicInteger id;
  private final ConnectionPool pool;

  public DBSenderDataSource() throws DataSourceException {
    super(DBSenderDataSource.class.getResourceAsStream("sender.properties"), "");
    
    pool = ConnectionPoolFactory.createConnectionPool("sender", Integer.MAX_VALUE, 60000);
    pool.init(1);

  }

  private int getNextId() throws DataSourceException {
    if (id == null) {
      try {
        idLock.lock();
        id= new AtomicInteger(loadId());
      } finally {
        idLock.unlock();
      }
    }
    return id.incrementAndGet();
  }

  private int loadId() throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("sender.message.load.max.id"));

      rs = ps.executeQuery();

      return (rs.next()) ? rs.getInt(1) : 0;

    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(rs, ps, conn);
    }

  }

  public SenderMessage loadSenderMessageById(int id) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("sender.message.load.by.id"));
      ps.setInt(1, id);
      rs = ps.executeQuery();

      if (rs != null && rs.next()) {
        final SenderMessage msg = new SenderMessage(rs.getInt(1));
        msg.setSmppStatus(rs.getInt(2));
        msg.setStatus(rs.getInt(3));
        return msg;
      }

      return null;

    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(rs, ps, conn);
    }

  }

  public void saveSenderMessage(SenderMessage msg) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = pool.getConnection();

      ps = conn.prepareStatement(getSql(msg.isExists() ? "sender.message.update" : "sender.message.insert"));

      if (!msg.isExists())
        msg.setId(getNextId());

      ps.setInt(1, msg.getSmppStatus());
      ps.setInt(2, msg.getStatus());
      ps.setLong(3, msg.getSmppId());
      ps.setInt(4, msg.getId());

      ps.executeUpdate();

    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }
  }

  public void removeSenderMessage(SenderMessage msg) throws DataSourceException {
    if (!msg.isExists())
      return;

    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = pool.getConnection();

      ps = conn.prepareStatement(getSql("sender.message.remove"));

      ps.setInt(1, msg.getId());

      ps.executeUpdate();

    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }
  }

  public int updateMessageStatus(long smppId, int status) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = pool.getConnection();

      ps = conn.prepareStatement(getSql("sender.message.update.by.smpp.id"));

      ps.setInt(1, status);
      ps.setLong(2, smppId);

      return ps.executeUpdate();

    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }
  }

  public void updateMessageSmppId(SenderMessage msg) throws DataSourceException {
    if (!msg.isExists())
      return;

    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = pool.getConnection();

      ps = conn.prepareStatement(getSql("sender.message.update.smpp.id"));

      ps.setLong(1, msg.getSmppId());
      ps.setInt(2, msg.getId());

      ps.executeUpdate();

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

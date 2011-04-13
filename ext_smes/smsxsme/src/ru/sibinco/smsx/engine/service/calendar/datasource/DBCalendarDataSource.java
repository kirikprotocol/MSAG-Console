package ru.sibinco.smsx.engine.service.calendar.datasource;

import java.sql.*;
import java.util.Collection;
import java.util.Date;
import java.util.LinkedList;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import ru.sibinco.smsx.network.dbconnection.ConnectionPool;
import ru.sibinco.smsx.network.dbconnection.ConnectionPoolFactory;
import ru.sibinco.smsx.utils.DBDataSource;
import ru.sibinco.smsx.utils.DataSourceException;

/**
 * User: artem
 * Date: Jun 30, 2007
 */

public class DBCalendarDataSource extends DBDataSource implements CalendarDataSource {

  private final ConnectionPool pool;
  private AtomicInteger id;
  private Lock idLock = new ReentrantLock();

  public DBCalendarDataSource() throws DataSourceException {
    super(DBCalendarDataSource.class.getResourceAsStream("calendar.properties"), "");

    pool = ConnectionPoolFactory.createConnectionPool("calendar", Integer.MAX_VALUE, 60000);
    pool.init(1);
  }

  private int getNextId() throws DataSourceException {
    if (id == null) {
      try {
        idLock.lock();
        if (id == null)
          id = new AtomicInteger(loadId());
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
      ps = conn.prepareStatement(getSql("calendar.message.load.max.id"));

      rs = ps.executeQuery();

      return (rs.next()) ? rs.getInt(1) : 0;

    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(rs, ps, conn);
    }

  }

  public Collection<CalendarMessage> loadCalendarMessages(Date toDate, int maxSize) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;

    final Collection<CalendarMessage> messagesList = new LinkedList<CalendarMessage>();

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("calendar.message.load.by.date"));

      ps.setTimestamp(1, new Timestamp(toDate.getTime()));
      ps.setInt(2, maxSize);
      rs = ps.executeQuery();

      CalendarMessage msg;
      while (rs != null && rs.next()) {
        msg = new CalendarMessage(rs.getInt(1));
        msg.setSourceAddress(rs.getString(2));
        msg.setDestinationAddress(rs.getString(3));
        msg.setSendDate(rs.getTimestamp(4));
        msg.setDestAddressSubunit(rs.getInt(5));
        msg.setMessage(rs.getString(6));
        msg.setStatus(rs.getInt(7));
        msg.setSaveDeliveryStatus(rs.getInt(8) == 1);
        msg.setSmppStatus(rs.getInt(9));
        msg.setConnectionName(rs.getString(10));
        msg.setMscAddress(rs.getString(11));
        msg.setAppendAdvertising(rs.getInt(12) == 1);

        messagesList.add(msg);
      }

    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(rs, ps, conn);
    }

    return messagesList;
  }

  public CalendarMessage loadCalendarMessageById(int id) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("calendar.message.load.by.id"));

      ps.setInt(1, id);
      rs = ps.executeQuery();

      if (rs != null && rs.next()) {
        final CalendarMessage msg = new CalendarMessage(rs.getInt(1));
        msg.setSourceAddress(rs.getString(2));
        msg.setDestinationAddress(rs.getString(3));
        msg.setSendDate(rs.getTimestamp(4));
        msg.setDestAddressSubunit(rs.getInt(5));
        msg.setMessage(rs.getString(6));
        msg.setStatus(rs.getInt(7));
        msg.setSaveDeliveryStatus(rs.getInt(8) == 1);
        msg.setSmppStatus(rs.getInt(9));
        msg.setConnectionName(rs.getString(10));
        msg.setMscAddress(rs.getString(11));
        msg.setAppendAdvertising(rs.getInt(12) == 1);
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

  public void removeCalendarMessage(CalendarMessage calendarMessage) throws DataSourceException {
    if (!calendarMessage.isExists())
      return;

    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("calendar.message.remove.by.id"));
      ps.setInt(1, calendarMessage.getId());

      ps.executeUpdate();
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }
  }

  public int removeCalendarMessages(Date maxDate, int limit) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("calendar.message.remove.by.date"));
      ps.setTimestamp(1, new Timestamp(maxDate.getTime()));
      ps.setInt(2, limit);

      return ps.executeUpdate();
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e);
    } finally {
      _close(null, ps, conn);
    }
  }

  public void saveCalendarMessage(CalendarMessage calendarMessage) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql(calendarMessage.isExists() ? "calendar.message.update" : "calendar.message.insert"));

      ps.setString(1, calendarMessage.getSourceAddress());
      ps.setString(2, calendarMessage.getDestinationAddress());
      ps.setTimestamp(3, calendarMessage.getSendDate());
      ps.setInt(4, calendarMessage.getDestAddressSubunit());
      ps.setString(5, calendarMessage.getMessage());
      ps.setInt(6, calendarMessage.getStatus());
      ps.setInt(7, calendarMessage.isSaveDeliveryStatus() ? 1 : 0);
      ps.setInt(8, calendarMessage.getSmppStatus());
      ps.setString(9, calendarMessage.getConnectionName());
      ps.setString(10, calendarMessage.getMscAddress());
      ps.setInt(11, calendarMessage.isAppendAdvertising() ? 1 : 0);

      if (!calendarMessage.isExists())
        calendarMessage.setId(getNextId());

      ps.setInt(12, calendarMessage.getId());


      ps.executeUpdate();
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e.getMessage());
    } finally {
      _close(null, ps, conn);
    }
  }

  public void updateMessageStatus(CalendarMessage calendarMessage) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("calendar.message.update.status.by.id"));

      ps.setInt(1, calendarMessage.getStatus());
      ps.setInt(2, calendarMessage.getId());

      ps.executeUpdate();
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e.getMessage());
    } finally {
      _close(null, ps, conn);
    }
  }

  public int updateMessageStatus(long id, int newStatus) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("calendar.message.update.status.by.smpp.id"));

      ps.setInt(1, newStatus);
      ps.setLong(2, id);

      return ps.executeUpdate();
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e.getMessage());
    } finally {
      _close(null, ps, conn);
    }
  }

  public void updateMessageSmppId(CalendarMessage calendarMessage) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("calendar.message.update.smpp.id"));

      ps.setLong(1, calendarMessage.getSmppId());
      ps.setInt(2, calendarMessage.getId());

      ps.executeUpdate();
    } catch (SQLException e) {
      pool.invalidateConnection(conn);
      throw new DataSourceException(e.getMessage());
    } finally {
      _close(null, ps, conn);
    }
  }

  public void release() {
    pool.release();
  }  
}

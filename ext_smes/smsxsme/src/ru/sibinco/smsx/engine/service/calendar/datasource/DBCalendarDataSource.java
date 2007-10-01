package ru.sibinco.smsx.engine.service.calendar.datasource;

import com.eyeline.sme.utils.ds.DBDataSource;
import com.eyeline.sme.utils.ds.DataSourceException;

import java.sql.*;
import java.util.Collection;
import java.util.Date;
import java.util.LinkedList;

import snaq.db.ConnectionPool;
import ru.sibinco.smsx.network.dbconnection.ConnectionPoolFactory;

/**
 * User: artem
 * Date: Jun 30, 2007
 */

public class DBCalendarDataSource extends DBDataSource implements CalendarDataSource {

  private static final Object idLock = new Object();
  private final ConnectionPool pool;
  private int id;

  public DBCalendarDataSource(String sqlFile, String prefix) throws DataSourceException {
    super(sqlFile, prefix);

    pool = ConnectionPoolFactory.createConnectionPool("calendar", 10, 60000);
    pool.init(1);

    synchronized (idLock) {
      this.id = loadId();
    }
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
      throw new DataSourceException(e);
    } finally {
      close(rs, ps, conn);
    }

  }

  public Collection loadCalendarMessages(Date toDate, int maxSize) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;

    final Collection messagesList = new LinkedList();

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

        messagesList.add(msg);
      }

    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(rs, ps, conn);
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
        return msg;
      }

      return null;

    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(rs, ps, conn);
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
      throw new DataSourceException(e);
    } finally {
      close(null, ps, conn);
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

      if (!calendarMessage.isExists()) {
        synchronized (idLock) {
          id++;
          calendarMessage.setId(id);
        }
      }

      ps.setInt(9, calendarMessage.getId());

      ps.executeUpdate();
    } catch (SQLException e) {
      throw new DataSourceException(e.getMessage());
    } finally {
      close(null, ps, conn);
    }
  }

  public void release() {
    pool.release();
  }
}

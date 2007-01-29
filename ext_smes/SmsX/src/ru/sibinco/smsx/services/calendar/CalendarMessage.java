package ru.sibinco.smsx.services.calendar;

import org.apache.log4j.Category;
import ru.sibinco.smsx.services.Storable;
import ru.sibinco.smsx.utils.ConnectionPool;

import java.sql.*;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

/**
 * User: artem
 * Date: Jul 31, 2006
 */

final class CalendarMessage extends Storable{

  private static final Category log = Category.getInstance(CalendarMessage.class);

  private final int id;
  private final String source;
  private final String dest;
  private final Timestamp sendDate;
  private final String message;

  public CalendarMessage(final int id, final String source, final String dest,
                         final Timestamp sendDate, final String message) {
    this.id = id;
    this.source = source;
    this.dest = dest;
    this.sendDate = sendDate;
    this.message = message;
  }

  public CalendarMessage(final String source, final String dest, final Date sendDate, final String message) {
    this(-1, source, dest, new Timestamp(sendDate.getTime()), message);
  }

  public void save() throws SQLException{
    Connection conn = null;
    try {
      conn = ConnectionPool.getConnection();
      save(conn);
    } catch (SQLException e) {
      log.error(e);
      throw new SQLException(e.getMessage());
    } finally {
      close(log, null, null, conn);
    }
  }

  public void save(Connection conn) throws SQLException {
    PreparedStatement ps = null;
    try {
      ps = conn.prepareStatement(CalendarService.Properties.CALENDAR_ENGINE_INSERT_MESSAGE_SQL);

      ps.setString(1, source);
      ps.setString(2, dest);
      ps.setTimestamp(3, sendDate);
      ps.setString(4, message);

      ps.executeUpdate();
    } catch (SQLException e) {
      log.error(e);
      throw new SQLException(e.getMessage());
    } finally {
      close(log, null, ps, null);
    }
  }



  public void remove() throws SQLException {
    Connection conn = null;
    try {
      conn = ConnectionPool.getConnection();
      remove(conn);
    } catch (SQLException e) {
      log.error(e);
      throw new SQLException(e.getMessage());
    } finally {
      close(log, null, null, conn);
    }
  }

  public void remove(Connection conn) throws SQLException {
    if (!isExists())
      return;

    PreparedStatement ps = null;
    try {
      ps = conn.prepareStatement(CalendarService.Properties.CALENDAR_ENGINE_REMOVE_MESSAGE_SQL);
      ps.setInt(1, id);

      ps.executeUpdate();
    } catch (SQLException e) {
      log.error(e);
      throw new SQLException(e.getMessage());
    } finally {
      close(log, null, ps, null);
    }
  }


  public static List loadList(Date toDate, int maxSize) throws SQLException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;

    final List messagesList = new ArrayList();

    try {
      conn = ConnectionPool.getConnection();
      ps = conn.prepareStatement(CalendarService.Properties.CALENDAR_ENGINE_LOAD_LIST_SQL);

      ps.setTimestamp(1, new Timestamp(toDate.getTime()));
      ps.setInt(2, maxSize);
      rs = ps.executeQuery();

      while (rs != null && rs.next())
        messagesList.add(new CalendarMessage(rs.getInt(1), rs.getString(2), rs.getString(3), rs.getTimestamp(4), rs.getString(5)));

      log.info("Loading messages list ok.");
    } catch (SQLException e) {
      log.error("Can't load messages list: ", e);
      throw new SQLException(e.getMessage());
    } finally {
      close(log, rs, ps, conn);
    }

    return messagesList;
  }

  public String getSource() {
    return source;
  }

  public String getDest() {
    return dest;
  }

  public Timestamp getSendDate() {
    return sendDate;
  }

  public String getMessage() {
    return message;
  }

  public int getId() {
    return id;
  }

  public boolean isExists() {
    return id != -1;
  }
}

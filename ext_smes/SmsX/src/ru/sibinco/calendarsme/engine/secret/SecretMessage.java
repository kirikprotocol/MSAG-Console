package ru.sibinco.calendarsme.engine.secret;

import ru.sibinco.calendarsme.engine.Storable;
import ru.sibinco.calendarsme.utils.Utils;
import ru.sibinco.calendarsme.utils.ConnectionPool;

import java.util.Properties;
import java.util.List;
import java.util.ArrayList;
import java.sql.*;

/**
 * User: artem
 * Date: Aug 2, 2006
 */

public class SecretMessage extends Storable {

  private static final org.apache.log4j.Category Log = org.apache.log4j.Category.getInstance(SecretRequestProcessor.class);

  private static final String insertMessageSQL;
  private static final String selectMessagesSQL;
  private static final String removeMessageSQL;

  static {
    final Properties config = Utils.loadConfig("secretmessage.properties");
    insertMessageSQL = Utils.loadString(config, "insert.sql");
    removeMessageSQL = Utils.loadString(config, "remove.sql");
    selectMessagesSQL = Utils.loadString(config, "select.by.user.sql");
    config.clear();
  }

  private int id = -1;
  private final String userNumber;
  private final String message;
  private final String fromNumber;
  private final Timestamp sendDate;

  public SecretMessage(String userNumber, String message, String fromNumber) {
    this(-1, userNumber, message, fromNumber, new Timestamp(new java.util.Date().getTime()));
  }

  public SecretMessage(final int id, final String userNumber, final String message, final String fromNumber, final Timestamp sendDate) {
    this.id = id;
    this.userNumber = userNumber;
    this.message = message;
    this.fromNumber = fromNumber;
    this.sendDate = sendDate;
  }

  public String getMessage() {
    return message;
  }

  public String getFromNumber() {
    return fromNumber;
  }

  public String getUserNumber() {
    return userNumber;
  }

  public int getId() {
    return id;
  }

  public Timestamp getSendDate() {
    return sendDate;
  }

  public boolean isExists() {
    return id != -1;
  }

  public void save() throws SQLException {
    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = ConnectionPool.getConnection();
      ps = conn.prepareStatement(insertMessageSQL);

      ps.setString(1, userNumber);
      ps.setString(2, fromNumber);
      ps.setString(3, message);

      ps.executeUpdate();

    } catch (SQLException e) {
      throw new SQLException(e.getMessage());
    } finally {
      close(Log, null, ps, conn);
    }
  }

  public void remove() throws SQLException {
    if (!isExists())
      return;

    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = ConnectionPool.getConnection();
      ps = conn.prepareStatement(removeMessageSQL);

      ps.setInt(1, id);

      ps.executeUpdate();

    } catch (SQLException e) {
      throw new SQLException(e.getMessage());
    } finally {
      close(Log, null, ps, conn);
    }
  }

  public static List loadByUser(final String userNumber) throws SQLException{
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;

    final List messages = new ArrayList();

    try {
      conn = ConnectionPool.getConnection();
      ps = conn.prepareStatement(selectMessagesSQL);

      ps.setString(1, userNumber);

      rs = ps.executeQuery();

      while (rs != null && rs.next())
        messages.add(new SecretMessage(rs.getInt(1), rs.getString(2), rs.getString(3), rs.getString(4), rs.getTimestamp(5)));

    } catch (SQLException e) {
      throw new SQLException(e.getMessage());
    } finally {
      close(Log, rs, ps, conn);
    }

    return messages;
  }
}

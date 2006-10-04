package ru.sibinco.calendarsme.engine.secret;

import ru.sibinco.calendarsme.engine.Storable;
import ru.sibinco.calendarsme.utils.Utils;
import ru.sibinco.calendarsme.utils.ConnectionPool;
import ru.sibinco.calendarsme.utils.MessageEncoder;

import java.util.Properties;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.ResultSet;

/**
 * User: artem
 * Date: Aug 2, 2006
 */

public class SecretUser extends Storable {

  private static final org.apache.log4j.Category Log = org.apache.log4j.Category.getInstance(SecretRequestProcessor.class);

  private static final String insertSQL;
  private static final String removeByNumberSQL;
  private static final String selectByNumberSQL;

  static {
    final Properties config = Utils.loadConfig("secretuser.properties");
    insertSQL = Utils.loadString(config, "insert.sql");
    removeByNumberSQL = Utils.loadString(config, "remove.sql");
    selectByNumberSQL = Utils.loadString(config, "select.by.number.sql");
  }

  private final String number;
  private final String password;

  public SecretUser(String number, String password) {
    this.number = number;
    this.password = password;
  }

  public String getNumber() {
    return number;
  }

  public String getPassword() {
    return password;
  }

  public boolean confirmPassword(final String pwd) throws MessageEncoder.EncodeException {
    return password.equals(pwd) || password.equals(MessageEncoder.encodeMD5(pwd));
  }

  public void save() throws SQLException, MessageEncoder.EncodeException {
    // remove old
    remove();

    // create new
    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = ConnectionPool.getConnection();
      ps = conn.prepareStatement(insertSQL);

      ps.setString(1, number);
      ps.setString(2, MessageEncoder.encodeMD5(password));

      ps.executeUpdate();

    } catch (SQLException e) {
      throw new SQLException(e.getMessage());
    } catch (MessageEncoder.EncodeException e) {
      throw new MessageEncoder.EncodeException(e);
    } finally {
      close(Log, null, ps, conn);
    }
  }

  public void remove() throws SQLException {
    if (number == null)
      return;

    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = ConnectionPool.getConnection();

      ps = conn.prepareStatement(removeByNumberSQL);
      ps.setString(1, number);

      ps.executeUpdate();

    } catch (SQLException e) {
      throw new SQLException(e.getMessage());
    } finally {
      close(Log, null, ps, conn);
    }
  }

  public static SecretUser loadByNumber(final String number) throws SQLException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;
    try {
      conn = ConnectionPool.getConnection();

      ps = conn.prepareStatement(selectByNumberSQL);
      ps.setString(1, number);

      rs = ps.executeQuery();
      return (rs != null && rs.next()) ? new SecretUser(rs.getString(1), rs.getString(2)) : null;
    } catch (SQLException e) {
      throw new SQLException(e.getMessage());
    } finally {
      close(Log, rs, ps, conn);
    }
  }


}

package ru.sibinco.calendarsme.engine.secret;

import ru.sibinco.calendarsme.SmeProperties;
import ru.sibinco.calendarsme.engine.Storable;
import ru.sibinco.calendarsme.utils.ConnectionPool;
import ru.sibinco.calendarsme.utils.MessageEncoder;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

/**
 * User: artem
 * Date: Aug 2, 2006
 */

public class SecretUser extends Storable {

  private static final org.apache.log4j.Category Log = org.apache.log4j.Category.getInstance(SecretRequestProcessor.class);

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

  public void updatePassword(final String password) throws SQLException, MessageEncoder.EncodeException {
    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = ConnectionPool.getConnection();
      ps = conn.prepareStatement(SmeProperties.SecretUser.UPDATE_PASSWORD_SQL);

      ps.setString(1, MessageEncoder.encodeMD5(password));
      ps.setString(2, number);

      ps.executeUpdate();

    } catch (SQLException e) {
      throw new SQLException(e.getMessage());
    } catch (MessageEncoder.EncodeException e) {
      throw new MessageEncoder.EncodeException(e);
    } finally {
      close(Log, null, ps, conn);
    }
  }

  public void save() throws SQLException, MessageEncoder.EncodeException {
    // remove old
    remove();

    // create new
    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = ConnectionPool.getConnection();
      ps = conn.prepareStatement(SmeProperties.SecretUser.INSERT_SQL);

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

      ps = conn.prepareStatement(SmeProperties.SecretUser.REMOVE_BY_NUMBER_SQL);
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

      ps = conn.prepareStatement(SmeProperties.SecretUser.SELECT_BY_NUMBER_SQL);
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

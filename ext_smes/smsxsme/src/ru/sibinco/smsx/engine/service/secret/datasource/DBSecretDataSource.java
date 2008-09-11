package ru.sibinco.smsx.engine.service.secret.datasource;

import java.sql.*;
import java.util.*;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

import snaq.db.ConnectionPool;
import ru.sibinco.smsx.network.dbconnection.ConnectionPoolFactory;
import ru.sibinco.smsx.utils.DBDataSource;
import ru.sibinco.smsx.utils.DataSourceException;

/**
 * User: artem
 * Date: Jul 1, 2007
 */

public class DBSecretDataSource extends DBDataSource implements SecretDataSource {

  private static final Object idLock = new Object();
  private int id;
  private final ConnectionPool pool;
  private final Matcher loadUserByAddresses;

  public DBSecretDataSource() throws DataSourceException {
    super(DBSecretDataSource.class.getResourceAsStream("secret.properties"), "");

    pool = ConnectionPoolFactory.createConnectionPool("secret", Integer.MAX_VALUE, 60000);
    pool.init(1);

    synchronized (idLock) {
      id = loadId();
    }

    loadUserByAddresses = Pattern.compile("0").matcher(getSql("secret.user.load.by.addresses"));
  }

  private int loadId() throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("secret.message.load.max.id"));

      rs = ps.executeQuery();

      return (rs.next()) ? rs.getInt(1) : 0;

    } catch (SQLException e) {
      throw new DataSourceException(e);
    } finally {
      close(rs, ps, conn);
    }

  }

  public SecretUser loadSecretUserByAddress(String address) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;
    try {
      conn = pool.getConnection();

      ps = conn.prepareStatement(getSql("secret.user.load.by.address"));
      ps.setString(1, address);

      rs = ps.executeQuery();
      SecretUser su = (rs != null && rs.next()) ? new SecretUser(rs.getString(1), rs.getString(2)) : null;
      if (su != null)
        su.setExists(true);
      return su;
    } catch (SQLException e) {
      throw new DataSourceException(e.getMessage());
    } finally {
      close(rs, ps, conn);
    }
  }

  public void saveSecretUser(SecretUser secretUser) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = pool.getConnection();

      if (!secretUser.isExists()) {
        ps = conn.prepareStatement(getSql("secret.user.insert"));

        ps.setString(1, secretUser.getAddress());
        ps.setString(2, secretUser.getPassword());

      } else {
        ps = conn.prepareStatement(getSql("secret.user.update"));

        ps.setString(1, secretUser.getPassword());
        ps.setString(2, secretUser.getAddress());
      }

      ps.executeUpdate();
      secretUser.setExists(true);

    } catch (SQLException e) {
      throw new DataSourceException(e.getMessage());
    } finally {
      close(null, ps, conn);
    }
  }

  public void removeSecretUser(SecretUser secretUser) throws DataSourceException {
    if (!secretUser.isExists())
      return;

    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = pool.getConnection();

      ps = conn.prepareStatement(getSql("secret.user.remove"));
      ps.setString(1, secretUser.getAddress());

      ps.executeUpdate();

      secretUser.setExists(false);

    } catch (SQLException e) {
      throw new DataSourceException(e.getMessage());
    } finally {
      close(null, ps, conn);
    }
  }

  public Map<String, SecretUser> loadSecretUsersByAddresses(String[] addresses) throws DataSourceException {

    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;

    try {
      conn = pool.getConnection();

      final StringBuilder buffer = new StringBuilder(addresses.length*12+addresses.length);
      for (int i=0; i<addresses.length; i++)
        buffer.append(buffer.length() > 0 ? "," : "").append('\'').append(addresses[i]).append('\'');

      ps = conn.prepareStatement(loadUserByAddresses.replaceAll(buffer.toString()));

      rs = ps.executeQuery();

      final Map<String, SecretUser> result = new HashMap<String, SecretUser>();

      while (rs!= null && rs.next())
        result.put(rs.getString(1), new SecretUser(rs.getString(1), rs.getString(2)));

      return result;

    } catch (SQLException e) {
      throw new DataSourceException(e.getMessage());
    } finally {
      close(rs, ps, conn);
    }
  }

  public Collection<SecretMessage> loadSecretMessagesByAddress(String address) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;

    final Collection<SecretMessage> messages = new LinkedList<SecretMessage>();

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("secret.message.load.by.user"));

      ps.setString(1, address);

      rs = ps.executeQuery();

      SecretMessage msg;
      while (rs != null && rs.next()) {
        msg = new SecretMessage(rs.getInt(1));
        msg.setDestinationAddress(rs.getString(2));
        msg.setMessage(rs.getString(3));
        msg.setSourceAddress(rs.getString(4));
        msg.setSendDate(rs.getTimestamp(5));
        msg.setDestAddressSubunit(rs.getInt(6));
        msg.setStatus(rs.getInt(7));
        msg.setSaveDeliveryStatus(rs.getInt(8) == 1);
        msg.setSmppStatus(rs.getInt(9));
        msg.setNotifyOriginator(rs.getInt(10) == 1);
        msg.setConnectionName(rs.getString(11));
        msg.setMscAddress(rs.getString(12));
        messages.add(msg);
      }

    } catch (SQLException e) {
      throw new DataSourceException(e.getMessage());
    } finally {
      close(rs, ps, conn);
    }

    return messages;
  }

  public SecretMessage loadSecretMessageById(int id) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("secret.message.load.by.id"));

      ps.setInt(1, id);

      rs = ps.executeQuery();

      if (rs != null && rs.next()) {
        final SecretMessage msg = new SecretMessage(rs.getInt(1));
        msg.setDestinationAddress(rs.getString(2));
        msg.setMessage(rs.getString(3));
        msg.setSourceAddress(rs.getString(4));
        msg.setSendDate(rs.getTimestamp(5));
        msg.setDestAddressSubunit(rs.getInt(6));
        msg.setStatus(rs.getInt(7));
        msg.setSaveDeliveryStatus(rs.getInt(8) == 1);
        msg.setSmppStatus(rs.getInt(9));
        msg.setNotifyOriginator(rs.getInt(10) == 1);
        msg.setConnectionName(rs.getString(11));
        msg.setMscAddress(rs.getString(12));
        return msg;
      }

      return null;

    } catch (SQLException e) {
      throw new DataSourceException(e.getMessage());
    } finally {
      close(rs, ps, conn);
    }
  }

  public int loadMessagesCountByAddress(String address) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("secret.message.load.count.by.user"));

      ps.setString(1, address);

      rs = ps.executeQuery();

      if (rs != null && rs.next())
        return rs.getInt(1);

      return -1;

    } catch (SQLException e) {
      throw new DataSourceException(e.getMessage());
    } finally {
      close(rs, ps, conn);
    }
  }

  public void saveSecretMessage(SecretMessage secretMessage) throws DataSourceException {
    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();

      ps = conn.prepareStatement(getSql((!secretMessage.isExists()) ? "secret.message.insert" : "secret.message.update"));

      ps.setString(1, secretMessage.getDestinationAddress());
      ps.setString(2, secretMessage.getSourceAddress());
      ps.setString(3, secretMessage.getMessage());
      ps.setInt(4, secretMessage.getDestAddressSubunit());
      ps.setInt(5, secretMessage.getStatus());
      ps.setInt(6, secretMessage.isSaveDeliveryStatus() ? 1 : 0);
      ps.setInt(7, secretMessage.getSmppStatus());
      ps.setInt(8, secretMessage.isNotifyOriginator() ? 1 : 0);
      ps.setString(9, secretMessage.getConnectionName());
      ps.setString(10, secretMessage.getMscAddress());

      if (!secretMessage.isExists()) {
        synchronized (idLock) {
          id++;
          secretMessage.setId(id);
        }
      }

      ps.setInt(11, secretMessage.getId());

      ps.executeUpdate();

    } catch (SQLException e) {
      throw new DataSourceException(e.getMessage());
    } finally {
      close(null, ps, conn);
    }
  }

  public void removeSecretMessage(SecretMessage secretMessage) throws DataSourceException {
    if (!secretMessage.isExists())
      return;

    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("secret.message.remove"));

      ps.setInt(1, secretMessage.getId());

      ps.executeUpdate();

    } catch (SQLException e) {
      throw new DataSourceException(e.getMessage());
    } finally {
      close(null, ps, conn);
    }
  }

  public void updateMessageStatus(SecretMessage secretMessage) throws DataSourceException {
    if (!secretMessage.isExists())
      return;

    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("secret.message.update.status.by.id"));

      ps.setInt(1, secretMessage.getStatus());
      ps.setInt(2, secretMessage.getId());

      ps.executeUpdate();

    } catch (SQLException e) {
      throw new DataSourceException(e.getMessage());
    } finally {
      close(null, ps, conn);
    }
  }

  public int updateMessageStatus(long smppId, int status) throws DataSourceException {

    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("secret.message.update.status.by.smpp.id"));

      ps.setInt(1, status);
      ps.setLong(2, smppId);

      return ps.executeUpdate();

    } catch (SQLException e) {
      throw new DataSourceException(e.getMessage());
    } finally {
      close(null, ps, conn);
    }
  }

  public void updateMessageSmppId(SecretMessage secretMessage) throws DataSourceException {
    if (!secretMessage.isExists())
      return;

    Connection conn = null;
    PreparedStatement ps = null;
    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("secret.message.update.smpp.id"));

      ps.setLong(1, secretMessage.getSmppId());
      ps.setInt(2, secretMessage.getId());

      ps.executeUpdate();

    } catch (SQLException e) {
      throw new DataSourceException(e.getMessage());
    } finally {
      close(null, ps, conn);
    }
  }

  public SecretUserWithMessages loadSecretUserWithMessages(String address) throws DataSourceException {

    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;

    try {
      conn = pool.getConnection();
      ps = conn.prepareStatement(getSql("secret.load.user.and.messages"));

      ps.setString(1, address);

      rs = ps.executeQuery();

      if (rs == null || !rs.next() || rs.getString(1) == null)
          return null;

      final SecretUser secretUser = new SecretUser(address, rs.getString(1));
      final Collection messages = new  LinkedList();

      SecretMessage msg;
      // Load message in first row
      if (rs.getString(2) != null) {
        msg = new SecretMessage(rs.getInt(2));
        msg.setDestinationAddress(rs.getString(3));
        msg.setMessage(rs.getString(4));
        msg.setSourceAddress(rs.getString(5));
        msg.setSendDate(rs.getTimestamp(6));
        msg.setDestAddressSubunit(rs.getInt(7));
        msg.setStatus(rs.getInt(8));
        msg.setSaveDeliveryStatus(rs.getInt(9) == 1);
        msg.setSmppStatus(rs.getInt(10));
        msg.setNotifyOriginator(rs.getInt(11) == 1);
        msg.setConnectionName(rs.getString(12));
        msg.setMscAddress(rs.getString(13));
        messages.add(msg);
      }

      // Load messages from other rows
      while (rs.next()) {
        msg = new SecretMessage(rs.getInt(2));
        msg.setDestinationAddress(rs.getString(3));
        msg.setMessage(rs.getString(4));
        msg.setSourceAddress(rs.getString(5));
        msg.setSendDate(rs.getTimestamp(6));
        msg.setDestAddressSubunit(rs.getInt(7));
        msg.setStatus(rs.getInt(8));
        msg.setSaveDeliveryStatus(rs.getInt(9) == 1);
        msg.setSmppStatus(rs.getInt(10));
        msg.setNotifyOriginator(rs.getInt(11) == 1);
        msg.setConnectionName(rs.getString(12));
        msg.setMscAddress(rs.getString(13));
        messages.add(msg);
      }

      return new SecretUserWithMessages(secretUser, messages);

    } catch (SQLException e) {
      throw new DataSourceException(e.getMessage());
    } finally {
      close(rs, ps, conn);
    }
  }

  public void release() {
    pool.release();
  }
}

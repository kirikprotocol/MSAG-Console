package ru.sibinco.smpp.cmb;

import java.sql.*;

import ru.sibinco.smpp.util.DBConnectionManager;
import ru.sibinco.smpp.InitializationException;

/**
 * Created by IntelliJ IDEA.
 * User: kpv
 * Date: Dec 22, 2006
 * Time: 4:12:40 PM
 */
public class MemoryConstraintManagerImpl extends AbstractConstraintManager {

  private MemoryCache cache = MemoryCache.getInstance();
  private DBConnectionManager dbManager = null;
  private String sql = null;

  public MemoryConstraintManagerImpl() throws InitializationException {
    super();
    dbManager = DBConnectionManager.getInstance();
    sql = config.getProperty("get.balance.sql").trim();
    if (sql == null || sql.length() == 0) {
      throw new InitializationException("Mandatory parameter \"get.balance.sql\" not specified.");
    }
    Connection conn = null;
    CallableStatement stmt = null;
    try {
      conn = dbManager.getConnectionFromPool(config.getProperty("get.balance.connection.pool", "balance"));
      stmt = conn.prepareCall(sql);
    } catch (SQLException e) {
      if (conn == null)
        throw new InitializationException("Could not get SQL connection from pool: "+config.getProperty("get.balance.connection.pool", "balance"), e);
      else
        throw new InitializationException("Could not prepare call by \"get.balance.sql\"="+sql, e);
    } finally {
      if (stmt != null) {
        try {
          stmt.close();
        } catch (SQLException e) {
          Logger.warn("Could not close callable statement.", e);
        }
      }
      if (conn != null) {
        try {
          conn.close();
        } catch (SQLException e) {
          Logger.warn("Could not close connection.", e);
        }
      }
    }
  }

  public short check(String abonent)
      throws CheckConstraintsException,
      AttemptsLimitReachedException,
      UsageLimitReachedException,
      BalanceLimitException {
    if (!checkBalance && !checkAttempts && !checkUsages)
      return 0x0000;
    int _abonent;
    try {
      _abonent = Integer.parseInt((abonent.length() > 7 ? abonent.substring(abonent.length()-7) : abonent));
    } catch (NumberFormatException e) {
      throw new CheckConstraintsException("Could not get cache for abonent: "+abonent, e);
    }
    String date = getCurrentRdate();
    short data;
    try {
      data = cache.getCache(_abonent);
    } catch (IndexOutOfBoundsException e) {
      throw new CheckConstraintsException("Could not get cache for abonent: "+abonent, e);
    }
    int u = (data>>8)&0xFF;
    int a = ((int)data)&0xFF;
    if (checkAttempts && a >= attemptsLimit)
      throw  new AttemptsLimitReachedException(abonent, attemptsLimit, date);
    if (checkUsages && u >= usagesLimit)
      throw new UsageLimitReachedException(abonent, usagesLimit, date);
    if (!(getBalance(abonent) <= 0))
      throw new BalanceLimitException(abonent);
    a++;
    data = (short)(u << 8 | a);
    try {
      cache.setCache(_abonent, data);
    } catch (IndexOutOfBoundsException e) {
      throw new CheckConstraintsException("Could not update cache for abonent: "+abonent, e);
    }
    return data;
  }

  public int registerUsage(String abonent) throws CheckConstraintsException {
    if (!checkUsages)
      return usagesLimit;
    int _abonent;
    try {
      _abonent = Integer.parseInt((abonent.length() > 7 ? abonent.substring(abonent.length()-7) : abonent));
    } catch (NumberFormatException e) {
      throw new CheckConstraintsException("Could not get cache for abonent: "+abonent, e);
    }
    short data;
    try {
      data = cache.getCache(_abonent);
    } catch (IndexOutOfBoundsException e) {
      throw new CheckConstraintsException("Could not get cache for abonent: "+abonent, e);
    }
    int a = ((int)data)&0xFF;
    int u = (data>>8)&0xFF;
    u++;
    try {
      cache.setCache(_abonent, (short)(u << 8 | a));
    } catch (IndexOutOfBoundsException e) {
      throw new CheckConstraintsException("Could not update cache for abonent: "+abonent, e);
    }
    return usagesLimit - u;
  }

  public int unregisterUsage(String abonent) throws CheckConstraintsException {
    if (!checkUsages)
      return usagesLimit;
    int _abonent;
    try {
      _abonent = Integer.parseInt((abonent.length() > 7 ? abonent.substring(abonent.length()-7) : abonent));
    } catch (NumberFormatException e) {
      throw new CheckConstraintsException("Could not get cache for abonent: "+abonent, e);
    }
    short data;
    try {
      data = cache.getCache(_abonent);
    } catch (IndexOutOfBoundsException e) {
      throw new CheckConstraintsException("Could not get cache for abonent: "+abonent, e);
    }
    int a = ((int)data)&0xFF;
    int u = (data>>8)&0xFF;
    u--;
    try {
      cache.setCache(_abonent, (short)(u << 8 | a));
    } catch (IndexOutOfBoundsException e) {
      throw new CheckConstraintsException("Could not update cache for abonent: "+abonent, e);
    }
    return usagesLimit - u;
  }

  public int getAttemptsLimit() {
    return attemptsLimit;
  }

  public int getUsagesLimit() {
    return usagesLimit;
  }

  private Connection getConnection() throws SQLException {
    return dbManager.getConnectionFromPool(config.getProperty("get.balance.connection.pool", "balance"));
  }

  private int getBalance(String abonent) throws CheckConstraintsException {
    if (!checkBalance)
      return 0;
    int result = 0;
    Connection connection = null;
    CallableStatement stmt = null;
    ResultSet rs = null;
    String number = abonent;
    if (number.charAt(0) == '+') number = number.substring(1);
    else if (number.charAt(0) == '.') number = number.substring(4);
    try {
      connection = getConnection();
      stmt = connection.prepareCall(sql);
      stmt.registerOutParameter(1, java.sql.Types.INTEGER);
      stmt.setString(2, number);
      rs = stmt.executeQuery();
      if (rs.next()) {
        result = stmt.getInt(1);
      }
    } catch (SQLException e) {
      throw new CheckConstraintsException("Could not get balance for abonent #"+abonent, e);
    } catch (NullPointerException e) {
      throw new CheckConstraintsException("Could not get balance for abonent #"+abonent+". Result is NULL!", e);
    } finally {
      if (rs != null)
        try {
          rs.close();
        } catch (SQLException e) {
          Logger.warn("Could not close ResultSet.", e);
        }
      if (stmt != null)
        try {
          stmt.close();
        } catch (SQLException e) {
          Logger.warn("Could not close CallableStatement.", e);
        }
      if (connection != null)
        try {
          connection.close();
        } catch (SQLException e) {
          Logger.warn("Could not close Connection.", e);
        }
    }
    return result;
  }
}

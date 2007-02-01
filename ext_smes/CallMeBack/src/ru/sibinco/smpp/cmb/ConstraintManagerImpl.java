package ru.sibinco.smpp.cmb;

import ru.sibinco.smpp.InitializationException;
import ru.sibinco.smpp.util.DBConnectionManager;

import java.util.Properties;
import java.util.Date;
import java.text.SimpleDateFormat;
import java.sql.Connection;
import java.sql.CallableStatement;
import java.sql.SQLException;
import java.sql.ResultSet;
import java.lang.reflect.Method;

/**
 * Created by IntelliJ IDEA.
 * User: kpv
 * Date: Dec 22, 2006
 * Time: 4:14:15 PM
 */
public class ConstraintManagerImpl implements ConstraintManager {

  private static org.apache.log4j.Category Logger = org.apache.log4j.Category.getInstance(ConstraintManager.class);

  private Properties config = null;
  private boolean checkBalance = false;
  private boolean checkAttempts = false;
  private boolean checkUsages = false;
  private int attemptsLimit = 0;
  private int usagesLimit = 0;
  private Cache cache = null;
  private DBConnectionManager dbManager = null;
  private String sql = null;
  private SimpleDateFormat sdf = new SimpleDateFormat("dd/MM/yy");

  public ConstraintManagerImpl() throws InitializationException {
    // load configuration
    try {
      config = new Properties();
      config.load(getClass().getClassLoader().getResourceAsStream("ConstraintManager.properties"));
    } catch (Exception e) {
      Logger.fatal("Could not load ConstraintManager configuration from ConstraintManager.properties: "+e.getMessage(), e);
      throw new InitializationException("Could not load ConstraintManager configuration from ConstraintManager.properties: "+e.getMessage(), e);
    }
    // load parameters
    if (config.getProperty("check.balance") == null) {
      Logger.warn("Mandatory parameter \"check.balance\" not specified in ConstraintManager.properties. Use FALSE by default.");
      config.setProperty("check.balance", "false");
    } else  if (!config.getProperty("check.balance").trim().equalsIgnoreCase("true") &&
        !config.getProperty("check.balance").trim().equalsIgnoreCase("false")) {
      Logger.warn("\"check.balance\" value is invalid: "+config.getProperty("check.balance")+" Use FALSE by default.");
      config.setProperty("check.balance", "false");
    } else {
      checkBalance = config.getProperty("check.balance").trim().equalsIgnoreCase("true");
    }
    if (config.getProperty("check.attempts") == null) {
      config.setProperty("check.attempts", "false");
      Logger.warn("Mandatory parameter \"check.attempts\" not specified in ConstraintManager.properties. Use FALSE by default.");
    } else  if (!config.getProperty("check.attempts").trim().equalsIgnoreCase("true") &&
        !config.getProperty("check.attempts").trim().equalsIgnoreCase("false")) {
      Logger.warn("\"check.attempts\" value is invalid: "+config.getProperty("check.attempts")+" Use FALSE by default.");
      config.setProperty("check.attempts", "false");
    } else {
      checkAttempts = config.getProperty("check.attempts").trim().equalsIgnoreCase("true");
    }
    if (config.getProperty("check.usages") == null) {
      config.setProperty("check.usages", "false");
      Logger.warn("Mandatory parameter \"check.usages\" not specified in ConstraintManager.properties. Use FALSE by default.");
    } else  if (!config.getProperty("check.usages").trim().equalsIgnoreCase("true") &&
        !config.getProperty("check.usages").trim().equalsIgnoreCase("false")) {
      Logger.warn("\"check.usages\" value is invalid: "+config.getProperty("check.usages")+" Use FALSE by default.");
      config.setProperty("check.usages", "false");
    } else {
      checkUsages = config.getProperty("check.usages").trim().equalsIgnoreCase("true");
    }
    if (config.getProperty("usages.limit") == null) {
      config.setProperty("usages.limit", "0");
      Logger.warn("Mandatory parameter \"usages.limit\" not specified in ConstraintManager.properties. Use 0 (unlimited) by default.");
    } else {
      try {
        usagesLimit = Integer.parseInt(config.getProperty("usages.limit"));
        if (usagesLimit < 0) {
          Logger.warn("\"usages.limit\" value is invalid: "+config.getProperty("usages.limit")+" Use 0 (unlimited) by default.");
          config.setProperty("usages.limit", "0");
        }
      } catch (NumberFormatException e) {
        Logger.warn("\"usages.limit\" value is invalid: "+config.getProperty("usages.limit")+" Use 0 (unlimited) by default.");
        config.setProperty("usages.limit", "0");
      }
    }
    if (config.getProperty("attempts.limit") == null) {
      config.setProperty("attempts.limit", "0");
      Logger.warn("Mandatory parameter \"attempts.limit\" not specified in ConstraintManager.properties. Use 0 (unlimited) by default.");
    } else {
      try {
        attemptsLimit = Integer.parseInt(config.getProperty("attempts.limit"));
        if (attemptsLimit < 0) {
          Logger.warn("\"attempts.limit\" value is invalid: "+config.getProperty("attempts.limit")+" Use 0 (unlimited) by default.");
          config.setProperty("attempts.limit", "0");
        }
      } catch (NumberFormatException e) {
        Logger.warn("\"attempts.limit\" value is invalid: "+config.getProperty("attempts.limit")+" Use 0 (unlimited) by default.");
        config.setProperty("attempts.limit", "0");
      }
    }
    // init cache
    String cacheClassName = config.getProperty("cache.class", "ru.sibinco.smpp.cmb.PersistentCache").trim();
    try {
      Class c = Class.forName(cacheClassName);
      try {
        Method getInstance = c.getMethod("getInstance", null);
        cache = (Cache) getInstance.invoke(c, null);
      } catch (NoSuchMethodException e) {
        cache = (Cache) c.newInstance();
      }
    } catch (Exception e) {
      throw new InitializationException("Could not initialize cache class: " + cacheClassName, e);
    }
    if (Logger.isInfoEnabled())
      Logger.info("Cache "+cacheClassName+" initialized.");
    // init DB manager
    dbManager = DBConnectionManager.getInstance();
    if (Logger.isInfoEnabled())
      Logger.info("DB Manager initialized.");
    // check SQL query
    sql = config.getProperty("get.balance.sql").trim();
    if (sql == null || sql.length() == 0) {
      throw new InitializationException("Mandatory parameter \"get.balance.sql\" not specified.");
    }
    Connection conn = null;
    CallableStatement stmt = null;
    try {
      conn = dbManager.getConnectionFromPool(config.getProperty("get.balance.connection.pool", "balance"));
      stmt = conn.prepareCall(sql);
      if (Logger.isInfoEnabled())
        Logger.info("SQL query "+sql+" checked.");
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

  public void check(String abonent)
      throws CheckConstraintsException,
      AttemptsLimitReachedException,
      UsageLimitReachedException,
      BalanceLimitException {
    if (!checkBalance && !checkAttempts && !checkUsages)
      return;
    String date = getCurrentRdate();
    short data = getCache(getAbonentNumber(abonent));
    int u = (data>>8)&0xFF;
    int a = ((int)data)&0xFF;
    if (checkAttempts && a >= attemptsLimit)
      throw  new AttemptsLimitReachedException(abonent, attemptsLimit, date);
    if (checkUsages && u >= usagesLimit)
      throw new UsageLimitReachedException(abonent, usagesLimit, date);
    if (!(getBalance(abonent) <= 0))
      throw new BalanceLimitException(abonent);
  }

  public int registerAttempt(String abonent) throws CheckConstraintsException {
    if (!checkAttempts)
      return attemptsLimit;
    int _abonent = getAbonentNumber(abonent);
    short data = getCache(_abonent);
    int a = ((int)data)&0xFF;
    int u = (data>>8)&0xFF;
    a++;
    setCache(_abonent, u, a);
    return attemptsLimit - u;
  }

  public int unregisterAttempt(String abonent) throws CheckConstraintsException {
    if (!checkAttempts)
      return attemptsLimit;
    int _abonent = getAbonentNumber(abonent);
    short data = getCache(_abonent);
    int a = ((int)data)&0xFF;
    int u = (data>>8)&0xFF;
    a--;
    setCache(_abonent, u, a);
    return attemptsLimit - u;
  }

  public int registerUsage(String abonent) throws CheckConstraintsException {
    if (!checkUsages)
      return usagesLimit;
    int _abonent = getAbonentNumber(abonent);
    short data = getCache(_abonent);
    int a = ((int)data)&0xFF;
    int u = (data>>8)&0xFF;
    u++;
    setCache(_abonent, u, a);
    return usagesLimit - u;
  }

  public int unregisterUsage(String abonent) throws CheckConstraintsException {
    if (!checkUsages)
      return usagesLimit;
    int _abonent = getAbonentNumber(abonent);
    short data = getCache(_abonent);
    int a = ((int)data)&0xFF;
    int u = (data>>8)&0xFF;
    u--;
    setCache(_abonent, u, a);
    return usagesLimit - u;
  }

  private int getAbonentNumber(String abonent) throws CheckConstraintsException {
    try {
      return Integer.parseInt((abonent.length() > 7 ? abonent.substring(abonent.length()-7) : abonent));
    } catch (NumberFormatException e) {
      throw new CheckConstraintsException("Could not get cache for abonent: "+abonent, e);
    }
  }

  private short getCache(int abonent) throws CheckConstraintsException {
    try {
      return cache.getCache(abonent);
    } catch (IndexOutOfBoundsException e) {
      throw new CheckConstraintsException("Could not get cache for abonent: "+abonent, e);
    }
  }

  private void setCache(int abonent, int usages, int attempts) throws CheckConstraintsException {
    try {
      cache.setCache(abonent, (short)(usages << 8 | attempts));
    } catch (IndexOutOfBoundsException e) {
      throw new CheckConstraintsException("Could not update cache for abonent: "+abonent, e);
    }
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

  private String getCurrentRdate() {
    return sdf.format(new Date(System.currentTimeMillis()));
  }
}

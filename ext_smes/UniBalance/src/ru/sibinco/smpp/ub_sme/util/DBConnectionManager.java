package ru.sibinco.smpp.ub_sme.util;

import ru.sibinco.smpp.ub_sme.InitializationException;
import ru.sibinco.util.conpool.ConnectionPool;

import java.io.IOException;
import java.sql.*;
import java.util.*;


public class DBConnectionManager {

  private static org.apache.log4j.Category Logger = org.apache.log4j.Category.getInstance(DBConnectionManager.class);

  private static final String defaultConfiguration = "connection.manager.properties";
  private static final String connectionPrefix = "connection.";
  private static final String poolPrefix = "pool.";

  private Properties config = null;
  private Map connections = new HashMap();
  private Map dataSources = new HashMap();

  private static DBConnectionManager instance = null;

  private DBConnectionManager() {
  }

  public static DBConnectionManager getInstance(String configFileName) throws InitializationException {
    if (instance == null) {
      instance = new DBConnectionManager();
      instance.init(configFileName);
    }
    return instance;
  }

  public static DBConnectionManager getInstance(Properties configuration) throws InitializationException {
    if (instance == null) {
      instance = new DBConnectionManager();
      instance.init(configuration);
    }
    return instance;
  }

  public static DBConnectionManager getInstance() throws InitializationException {
    return getInstance(defaultConfiguration);
  }

  private void init(String configuration) throws InitializationException {
    if (configuration == null || configuration.trim().length() == 0)
      throw new InitializationException("Could not load configuration from NULL.");
    Properties properties = new Properties();
    try {
      properties.load(getClass().getClassLoader().getResourceAsStream(configuration));
    } catch (IOException e) {
      Logger.error("Could not load " + configuration + " configuration file.", e);
      throw new ru.sibinco.smpp.ub_sme.InitializationException("Could not load " + configuration + " configuration file.", e);
    }
    init(properties);
  }

  private void init(Properties properties) throws InitializationException {
    if (properties == null)
      throw new InitializationException("Could not load configuration from NULL.");
    config = properties;
    List c = new LinkedList();
    List p = new LinkedList();
    for (Enumeration enumeration = config.keys(); enumeration.hasMoreElements();) {
      String key = (String) enumeration.nextElement();
      if (key.startsWith(connectionPrefix)) {
        String name = key.substring(connectionPrefix.length(), key.indexOf(".", connectionPrefix.length() + 1));
        if (c.indexOf(name) == -1) {
          c.add(name);
        }
      } else if (key.startsWith(poolPrefix)) {
        String name = key.substring(poolPrefix.length(), key.indexOf(".", poolPrefix.length() + 1));
        if (p.indexOf(name) == -1) {
          p.add(name);
        }
      }
    }
    for (Iterator iterator = c.iterator(); iterator.hasNext();) {
      String name = (String) iterator.next();
      if (Logger.isInfoEnabled()) Logger.info("Preparing " + name + " connection ...");
      try {
        ConnectionData connectionData = new ConnectionData(name, config);
        connections.put(name, connectionData);
        if (Logger.isInfoEnabled()) Logger.info("Prepared " + connectionData.toString());
      } catch (Exception e) {
        Logger.warn("Could not prepare connection " + name, e);
      }
    }
    for (Iterator iterator = p.iterator(); iterator.hasNext();) {
      String name = (String) iterator.next();
      if (Logger.isInfoEnabled()) Logger.info("Preparing " + name + " data source ...");
      try {
        DatasourceData datasourceData = new DatasourceData(name, config);
        dataSources.put(name, datasourceData);
        if (Logger.isInfoEnabled()) Logger.info("Prepared " + datasourceData.toString());
      } catch (Exception e) {
        Logger.warn("Could not prepare pool. " + name, e);
      }
    }
    if (Logger.isInfoEnabled()) Logger.info("DBConnectionManager initialized.");
  }

  protected ConnectionData getConnectionData(String name) {
    return (ConnectionData) connections.get(name);
  }

  public boolean hasConnection(String name) {
    if (name == null)
      return false;
    return connections.get(name) != null;
  }

  public boolean hasPool(String name) {
    if (name == null)
      return false;
    return dataSources.get(name) != null;
  }

  public Connection getConnection(String name) throws SQLException {
    if (name == null)
      return null;
    if (connections.get(name) == null)
      throw new SQLException("Connection \"" + name + "\" not found.");
    return ((ConnectionData) connections.get(name)).getConnection();
  }

  public Connection getConnectionFromPool(String name) throws SQLException {
    if (name == null)
      return null;
    if (dataSources.get(name) == null)
      throw new SQLException("Pool \"" + name + "\" not found.");
    return ((DatasourceData) dataSources.get(name)).getConnection();
  }

  class ConnectionData {
    private static final String driverSuffix = ".jdbc.driver";
    private static final String sourceSuffix = ".jdbc.source";
    private static final String userSuffix = ".jdbc.user";
    private static final String passwordSuffix = ".jdbc.password";
    private static final String dbTypeSuffix = ".type";
    private static final String dataSourceClassSuffix = ".data.source.class";

    private String name = null;
    private String driver = null;
    private String source = null;
    private String user = null;
    private String password = null;
    private String type = null;
    private String dataSourceClass = null;

    public ConnectionData(String name, Properties properties) throws InitializationException, IllegalArgumentException {
      if (name == null || name.trim().length() == 0)
        throw new IllegalArgumentException("Could not get connection configuration by NULL name.");
      this.name = name;

      if (properties == null)
        throw new IllegalArgumentException("Could not get connection \"" + getName() + "\" configuration from NULL.");

      init(properties);
    }

    private void init(Properties properties) throws InitializationException, IllegalArgumentException {
      if ((this.driver = properties.getProperty(connectionPrefix + getName() + driverSuffix)) == null ||
          this.driver.length() == 0) {
        throw new IllegalArgumentException("Could not prepare connection \"" + getName() + "\" with NULL driver.");
      }

      if ((this.source = properties.getProperty(connectionPrefix + getName() + sourceSuffix)) == null ||
          this.source.length() == 0) {
        throw new IllegalArgumentException("Could not prepare connection \"" + getName() + "\" with NULL source.");
      }

      if ((this.user = properties.getProperty(connectionPrefix + getName() + userSuffix)) == null ||
          this.user.length() == 0) {
        throw new IllegalArgumentException("Could not prepare connection \"" + getName() + "\" with NULL user.");
      }

      if ((this.password = properties.getProperty(connectionPrefix + getName() + passwordSuffix)) == null ||
          this.password.length() == 0) {
        throw new IllegalArgumentException("Could not prepare connection \"" + getName() + "\" with NULL password.");
      }

      if ((this.type = properties.getProperty(connectionPrefix + getName() + dbTypeSuffix)) == null ||
          this.type.length() == 0) {
        throw new IllegalArgumentException("Could not prepare connection \"" + getName() + "\" with NULL type.");
      }

      this.dataSourceClass = properties.getProperty(connectionPrefix + getName() + dataSourceClassSuffix);
      //check();
    }

    private void check() throws InitializationException, IllegalArgumentException {
      try {
        Class.forName(getDriver());
      } catch (ClassNotFoundException e) {
        Logger.error("Could not prepare connection \"" + getName() + "\". Illegal driver. Reason: " + e.getMessage(), e);
        throw new IllegalArgumentException("Could not prepare connection \"" + getName() + "\". Illegal driver. Reason: " + e.getMessage());
      }
      Connection connection = null;
      try {
        connection = DriverManager.getConnection(getSource(), getUser(), getPassword());
      } catch (SQLException e) {
        Logger.error("Could not prepare connection \"" + getName() + "\". Illegal connection configuration. Reason: " + e.getMessage(), e);
      } catch (Exception e) {
        throw new InitializationException("Could not prepare connection \"" + getName() + "\". Illegal connection configuration. Reason: " + e.getMessage());
      } finally {
        if (connection != null) {
          try {
            connection.close();
          } catch (SQLException e) {
            Logger.warn("Could not close test connection.", e);
          }
        }
      }
    }

    public String getName() {
      return name;
    }

    public String getDriver() {
      return driver;
    }

    public String getSource() {
      return source;
    }

    public String getUser() {
      return user;
    }

    public String getPassword() {
      return password;
    }

    public String getType() {
      return type;
    }

    public String getDataSourceClass() {
      return dataSourceClass;
    }

    public Connection getConnection() throws SQLException {
      return DriverManager.getConnection(getSource(), getUser(), getPassword());
    }

    public String toString() {
      StringBuffer sb = new StringBuffer("ConnectionData: ");
      sb.append("name=" + getName() + "; ");
      sb.append("driver=" + getDriver() + "; ");
      sb.append("source=" + getSource() + "; ");
      sb.append("user=" + getUser() + "; ");
      sb.append("password=" + getPassword() + "; ");
      sb.append("type=" + getType() + ".");
      return sb.toString();
    }
  }

  class DatasourceData {
    private static final String connectionSuffix = ".connection";
    private static final String sizeSuffix = ".size";
    private static final String timeoutSuffix = ".get.connection.timeout";
    private static final String checkSuffix = ".get.connection.check";
    private static final String validateSuffix = ".get.connection.validate";
    private static final String fillFactorSuffix = ".fill.factor";

    // for new connections pool
    private static final String suffix = ".jdbc";
    private static final String driverSuffix = ".driver";
    private static final String sourceSuffix = ".source";
    private static final String userSuffix = ".user";
    private static final String passwordSuffix = ".password";
    private static final String typeSuffix = ".pool.type";
    private static final String dataSourceClassSuffix = ".data.source.class";
    private static final String maxSizeSuffix = ".max.connections";
    private static final String minSizeSuffix = ".min.connections";
    private static final String idleSuffix = ".max.idle.time";
    private static final String shutdownSuffix = ".max.shutdown.time";
    private static final String loginSuffix = ".max.login.time";
    private static final String checkConnectionsSuffix = ".check.connections";
    private static final String checkIntervalSuffix = ".check.interval";

    private String name = null;
    private String connectionName = null;
    private int size = 5;
    //    private int timeout = 30;
    private boolean check = true;
    //    private boolean validate = false;
    private float fillFactor = 0;

    private int minSize = -1;
    private long idleTimeout = -1;
    private long shutdownTimeout = -1;
    private long loginTimeout = -1;
    private long checkInterval = 5000L;


    private Properties config = null;

    private ConnectionPool pool = null;

    protected DatasourceData(String name, Properties properties) throws InitializationException, IllegalArgumentException {
      if (name == null || name.trim().length() == 0)
        throw new IllegalArgumentException("Could not get pool configuration by NULL name.");
      this.name = name;

      if (properties == null)
        throw new IllegalArgumentException("Could not get pool \"" + getName() + "\" configuration from NULL.");

      init(properties);
    }

    private void init(Properties properties) throws InitializationException, IllegalArgumentException {
      if ((this.connectionName = properties.getProperty(poolPrefix + getName() + connectionSuffix)) == null ||
          getConnectionName().length() == 0 ||
          getConnectionData(getConnectionName()) == null) {
        throw new IllegalArgumentException("Could not prepare pool \"" + getName() + "\" with NULL connection.");
      }

      if (properties.getProperty(poolPrefix + getName() + sizeSuffix) == null ||
          properties.getProperty(poolPrefix + getName() + sizeSuffix).length() == 0) {
        throw new IllegalArgumentException("Could not prepare pool \"" + getName() + "\" with NULL size.");
      }
      try {
        this.size = Integer.parseInt(properties.getProperty(poolPrefix + getName() + sizeSuffix));
      } catch (NumberFormatException e) {
        throw new IllegalArgumentException("Could not prepare pool \"" + getName() + "\". Invalid size: " + properties.getProperty(poolPrefix + getName() + sizeSuffix));
      }

//      if (properties.getProperty(poolPrefix+getName()+timeoutSuffix) == null ||
//          properties.getProperty(poolPrefix+getName()+timeoutSuffix).length() == 0) {
//        throw new IllegalArgumentException("Could not prepare pool \""+getName()+"\" with NULL get connection timeout.");
//      }
//      try {
//        this.timeout = Integer.parseInt(properties.getProperty(poolPrefix+getName()+timeoutSuffix));
//      } catch (NumberFormatException e) {
//        throw new IllegalArgumentException("Could not prepare pool \""+getName()+"\". Invalid get connection timeout: "+properties.getProperty(poolPrefix+getName()+timeoutSuffix));
//      }

      if (properties.getProperty(poolPrefix + getName() + checkSuffix) == null ||
          properties.getProperty(poolPrefix + getName() + checkSuffix).length() == 0) {
        throw new IllegalArgumentException("Could not prepare pool \"" + getName() + "\" with NULL get connection check.");
      }
      this.check = properties.getProperty(poolPrefix + getName() + checkSuffix).trim().equalsIgnoreCase("1") ||
          properties.getProperty(poolPrefix + getName() + checkSuffix).trim().equalsIgnoreCase("yes") ||
          properties.getProperty(poolPrefix + getName() + checkSuffix).trim().equalsIgnoreCase("true");

//      if (properties.getProperty(poolPrefix+getName()+validateSuffix) != null &&
//            properties.getProperty(poolPrefix+getName()+validateSuffix).length() > 0) {
//      this.validate = properties.getProperty(poolPrefix+getName()+validateSuffix).trim().equalsIgnoreCase("1") ||
//                     properties.getProperty(poolPrefix+getName()+validateSuffix).trim().equalsIgnoreCase("yes") ||
//                     properties.getProperty(poolPrefix+getName()+validateSuffix).trim().equalsIgnoreCase("true");
//      }
      if (properties.getProperty(poolPrefix + getName() + fillFactorSuffix) == null ||
          properties.getProperty(poolPrefix + getName() + fillFactorSuffix).length() == 0) {
        throw new IllegalArgumentException("Could not prepare pool \"" + getName() + "\" with NULL fill factor.");
      }
      try {
        this.fillFactor = Float.parseFloat(properties.getProperty(poolPrefix + getName() + fillFactorSuffix));
      } catch (NumberFormatException e) {
        throw new IllegalArgumentException("Could not prepare pool \"" + getName() + "\". Invalid fill factor: " + properties.getProperty(poolPrefix + getName() + fillFactorSuffix));
      }
      if (fillFactor < 0.0 || fillFactor > 1.0)
        throw new IllegalArgumentException("Could not prepare pool \"" + getName() + "\". Invalid fill factor: " + fillFactor);

      minSize = (int) (size * fillFactor);
      if (properties.getProperty(poolPrefix + getName() + idleSuffix) != null &&
          properties.getProperty(poolPrefix + getName() + idleSuffix).length() != 0) {
        try {
          this.idleTimeout = Long.parseLong(properties.getProperty(poolPrefix + getName() + idleSuffix));
        } catch (NumberFormatException e) {
          throw new IllegalArgumentException("Could not prepare pool \"" + getName() + "\". Invalid idle timeout: " + properties.getProperty(poolPrefix + getName() + idleSuffix));
        }
      }
      if (properties.getProperty(poolPrefix + getName() + loginSuffix) != null &&
          properties.getProperty(poolPrefix + getName() + loginSuffix).length() != 0) {
        try {
          this.shutdownTimeout = Long.parseLong(properties.getProperty(poolPrefix + getName() + loginSuffix));
        } catch (NumberFormatException e) {
          throw new IllegalArgumentException("Could not prepare pool \"" + getName() + "\". Invalid idle timeout: " + properties.getProperty(poolPrefix + getName() + shutdownSuffix));
        }
      }
      if (properties.getProperty(poolPrefix + getName() + shutdownSuffix) != null &&
          properties.getProperty(poolPrefix + getName() + shutdownSuffix).length() != 0) {
        try {
          this.loginTimeout = Long.parseLong(properties.getProperty(poolPrefix + getName() + shutdownSuffix));
        } catch (NumberFormatException e) {
          throw new IllegalArgumentException("Could not prepare pool \"" + getName() + "\". Invalid idle timeout: " + properties.getProperty(poolPrefix + getName() + shutdownSuffix));
        }
      }
      if (properties.getProperty(poolPrefix + getName() + checkIntervalSuffix) != null &&
          properties.getProperty(poolPrefix + getName() + checkIntervalSuffix).length() != 0) {
        try {
          this.checkInterval = Long.parseLong(properties.getProperty(poolPrefix + getName() + checkIntervalSuffix));
        } catch (NumberFormatException e) {
          throw new IllegalArgumentException("Could not prepare pool \"" + getName() + "\". Invalid idle timeout: " + properties.getProperty(poolPrefix + getName() + checkIntervalSuffix));
        }
      }
      config = new Properties();
      config.setProperty(getName() + suffix + ".pool.name", getName());
      config.setProperty(getName() + suffix + driverSuffix, getConnectionData(getConnectionName()).getDriver());
      config.setProperty(getName() + suffix + sourceSuffix, getConnectionData(getConnectionName()).getSource());
      config.setProperty(getName() + suffix + userSuffix, getConnectionData(getConnectionName()).getUser());
      config.setProperty(getName() + suffix + passwordSuffix, getConnectionData(getConnectionName()).getPassword());
      config.setProperty(getName() + suffix + typeSuffix, getConnectionData(getConnectionName()).getType());
      if (getConnectionData(getConnectionName()).getDataSourceClass() != null) {
        config.setProperty(getName() + suffix + dataSourceClassSuffix, getConnectionData(getConnectionName()).getDataSourceClass());
      }
      config.setProperty(getName() + suffix + maxSizeSuffix, Integer.toString(getSize()));
      config.setProperty(getName() + suffix + minSizeSuffix, Integer.toString(getMinSize()));
      if (idleTimeout > 0) config.setProperty(getName() + suffix + idleSuffix, Long.toString(getIdleTimeout()));
      if (shutdownTimeout > 0)
        config.setProperty(getName() + suffix + shutdownSuffix, Long.toString(getShutdownTimeout()));
      if (loginTimeout > 0) config.setProperty(getName() + suffix + loginSuffix, Long.toString(getLoginTimeout()));
      config.setProperty(getName() + suffix + checkConnectionsSuffix, (isCheck() ? "true" : "false"));
      if (checkInterval > 0)
        config.setProperty(getName() + suffix + checkIntervalSuffix, Long.toString(getCheckInterval()));

      if (Logger.isInfoEnabled()) {
        for (Enumeration e = config.keys(); e.hasMoreElements();) {
          String key = (String) e.nextElement();
          Logger.info(key + "=" + config.getProperty(key));
        }
      }

      try {
        pool = new ConnectionPool(config, getName() + suffix);
      } catch (Exception e) {
        Logger.error("Coud not prepare pool \"" + getName() + "\".", e);
        throw new InitializationException("Coud not prepare pool \"" + getName() + "\".", e);
      }
    }

    public String getName() {
      return name;
    }

    public String getConnectionName() {
      return connectionName;
    }

    public int getSize() {
      return size;
    }

    public int getMinSize() {
      return minSize;
    }


    public long getIdleTimeout() {
      return idleTimeout;
    }

    public long getShutdownTimeout() {
      return shutdownTimeout;
    }

    public long getLoginTimeout() {
      return loginTimeout;
    }

    public boolean isCheck() {
      return check;
    }

    public long getCheckInterval() {
      return checkInterval;
    }

    public Connection getConnection() throws SQLException {
      return pool.getConnection();
    }

    public String toString() {
      StringBuffer sb = new StringBuffer("Pool: ");
      sb.append("name=" + getName() + "; ");
      sb.append("connection=" + getConnectionName() + "; ");
      sb.append("size=" + Integer.toString(getSize()) + "; ");
      sb.append("minSize=" + Integer.toString(getMinSize()) + "; ");
      sb.append("check=" + (isCheck() ? "true" : "false") + "; ");
      if (getCheckInterval() > 0) sb.append("checkInterval=" + Long.toString(getCheckInterval()) + "; ");
      if (getIdleTimeout() > 0) sb.append("idleTimeout=" + Long.toString(getIdleTimeout()) + "; ");
      if (getShutdownTimeout() > 0) sb.append("shutdownTimeout=" + Long.toString(getShutdownTimeout()) + "; ");
      if (getLoginTimeout() > 0) sb.append("loginTimeout=" + Long.toString(getLoginTimeout()) + ".");
      return sb.toString().trim();
    }
  }
}

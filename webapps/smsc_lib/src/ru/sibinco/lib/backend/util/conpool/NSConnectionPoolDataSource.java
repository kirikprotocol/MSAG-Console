package ru.sibinco.lib.backend.util.conpool;

import javax.sql.ConnectionPoolDataSource;
import javax.sql.PooledConnection;
import java.sql.SQLException;
import java.util.Properties;


public class NSConnectionPoolDataSource implements ConnectionPoolDataSource
{
  private static final int DEFAULT_LOGIN_TIMEOUT = 60;

  private String jdbcSource;
  private Properties props;
  private int loginTimeout = DEFAULT_LOGIN_TIMEOUT;
  private java.io.PrintWriter logWriter;

  public NSConnectionPoolDataSource(final String jdbcSource, final Properties props)
  {
    this.jdbcSource = jdbcSource;
    this.props = props;
  }

  public PooledConnection getPooledConnection() throws SQLException
  {
    return new NSPooledConnection(jdbcSource, props);
  }

  public PooledConnection getPooledConnection(final String user, final String password) throws SQLException
  {
    final Properties p1 = (Properties) props.clone();
    p1.put("user", user);
    p1.put("password", password);
    return new NSPooledConnection(jdbcSource, props);
  }

  public int getLoginTimeout() throws SQLException
  {
    return loginTimeout;
  }

  public void setLoginTimeout(final int val) throws SQLException
  {
    loginTimeout = val;
  }

  public java.io.PrintWriter getLogWriter() throws SQLException
  {
    return logWriter;
  }

  public void setLogWriter(final java.io.PrintWriter lw) throws SQLException
  {
    logWriter = lw;
  }
}

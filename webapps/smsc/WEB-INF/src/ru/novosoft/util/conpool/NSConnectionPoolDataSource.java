package ru.novosoft.util.conpool;

import javax.sql.*;
import java.sql.SQLException;
import java.util.Properties;

public class NSConnectionPoolDataSource implements ConnectionPoolDataSource {
  String jdbcSource;
  Properties props;
  int loginTimeout = 60;
  java.io.PrintWriter logWriter;

  public NSConnectionPoolDataSource( String jdbcSource, Properties props ) {
    this.jdbcSource = jdbcSource;
    this.props = props;
  }

  public javax.sql.PooledConnection getPooledConnection() throws SQLException {
    return new NSPooledConnection(jdbcSource, props );
  }

  public javax.sql.PooledConnection getPooledConnection( String user, String password )  throws SQLException {
    Properties p1 = (Properties)props.clone();
    p1.put( "user", user );
    p1.put( "password", password );
    return new NSPooledConnection(jdbcSource, props );
  }

  public int getLoginTimeout()  throws SQLException {
    return loginTimeout;
  }

  public void setLoginTimeout(int val)  throws SQLException {
    loginTimeout = val;
  }

  public java.io.PrintWriter getLogWriter()  throws SQLException {
    return logWriter;
  }

  public void setLogWriter(java.io.PrintWriter lw)  throws SQLException {
    logWriter = lw;
  }
}

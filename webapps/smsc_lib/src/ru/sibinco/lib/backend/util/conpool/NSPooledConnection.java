package ru.sibinco.lib.backend.util.conpool;

import org.apache.log4j.Category;

import javax.sql.ConnectionEvent;
import javax.sql.ConnectionEventListener;
import java.sql.*;
import java.util.Properties;


public class NSPooledConnection implements javax.sql.PooledConnection
{
  public static Category logger = Category.getInstance("ru.novosoft.util.conpool");

  private NSReusableConnection con = null;
  private String dburl = null;
  private Properties dbprops = null;

  private ConnectionEventListener[] listeners;
  private int listenersNum;

  private ConnectionEvent closeEvent = null;

  protected Exception trace = null;
  protected long lastUsed = System.currentTimeMillis();

  public NSPooledConnection(final String dburl, final String dbuser, final String dbpass) throws SQLException
  {
    final Properties props = new Properties();
    props.put("user", dbuser);
    props.put("password", dbpass);
    init(dburl, props);
  }

  public NSPooledConnection(final String dburl, final Properties props) throws SQLException
  {
    init(dburl, props);
  }

  protected void init(final String dburl, final Properties props) throws SQLException
  {
    dbprops = props;
    con = new NSReusableConnection(DriverManager.getConnection(dburl, dbprops), this);
    listeners = new ConnectionEventListener[1];
    closeEvent = new ConnectionEvent(this);
  }

  public Connection getConnection()
  {
    return con;
  }

  public void close() throws SQLException
  {
    con.realClose();
  }

  protected void free()
  {
    for (int i = 0; i < listenersNum; i++) {
      listeners[i].connectionClosed(closeEvent);
    }
  }

  public void addConnectionEventListener(final ConnectionEventListener listener)
  {
    synchronized (listeners) {
      if (listenersNum >= listeners.length) {
        final ConnectionEventListener[] new_listeners = new ConnectionEventListener[listeners.length + 1];
        System.arraycopy(listeners, 0, new_listeners, 0, listeners.length);
        listeners = new_listeners;
      }
      listeners[listenersNum++] = listener;
    }
  }

  public void removeConnectionEventListener(final ConnectionEventListener listener)
  {
    synchronized (listeners) {
      for (int i = 0; i < listeners.length; i++) {
        if (listener == listeners[i]) {
          final int tsz = listeners.length - i - 1;
          if (0 < tsz) System.arraycopy(listeners, i + 1, listeners, i, tsz);
          listenersNum--;
          return;
        }
      }
    }
  }

  public void lock()
  {
    lastUsed = System.currentTimeMillis();
    if (logger.isDebugEnabled()) {
      trace = new Exception("NSPooledConnection trace");
    }
  }

  public void unlock()
  {
    lastUsed = System.currentTimeMillis();
    trace = null;
  }
}


package ru.novosoft.util.conpool;

import java.sql.*;
import javax.sql.*;
import java.util.Properties;

public class NSPooledConnection implements javax.sql.PooledConnection {
  public static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance("ru.novosoft.util.conpool");
  NSReusableConnection con = null;
  String dburl = null;
  Properties dbprops = null;

  ConnectionEventListener listeners[];
  int listenersNum;

  ConnectionEvent closeEvent = null;

  protected Exception trace = null;
  protected long lastUsed = System.currentTimeMillis();

  public NSPooledConnection( String dburl, String dbuser, String dbpass ) throws SQLException {
    Properties props = new Properties();
    props.put( "user", dbuser );
    props.put( "password", dbpass );
    init(dburl,props); 
  }

  public NSPooledConnection( String dburl, Properties props ) throws SQLException {
    init(dburl,props); 
  }

  protected void init( String dburl, Properties props ) throws SQLException {
    dbprops = props;
    con = new NSReusableConnection( DriverManager.getConnection( dburl,dbprops ), this );
    listeners = new ConnectionEventListener[1];
    closeEvent = new ConnectionEvent( this );
  }

  public java.sql.Connection getConnection() {
    return con;
  }

  public void close() throws SQLException {
    con.realClose();
  }

  protected void free() {
    for (int i = 0; i < listenersNum; i++) {
      listeners[i].connectionClosed(closeEvent);
    }
  }

  public void addConnectionEventListener(ConnectionEventListener listener) {
    synchronized (listeners) {
      if( listenersNum >= listeners.length ) {
        ConnectionEventListener new_listeners[] = new ConnectionEventListener[listeners.length+1];
        System.arraycopy(listeners, 0, new_listeners, 0, listeners.length );
        listeners = new_listeners;
      }
      listeners[listenersNum++] = listener;
    }
  }

  public void removeConnectionEventListener(ConnectionEventListener listener) {
    synchronized (listeners) {
      for (int i = 0; i < listeners.length; i++) {
        if( listener == listeners[i] ) {
          int tsz = listeners.length-i-1;
          if( tsz > 0 ) System.arraycopy( listeners, i+1, listeners, i, tsz );
          listenersNum--;
          return;
        }
      }
    }
  }

  public void lock() {
    lastUsed = System.currentTimeMillis();
    if( logger.isDebugEnabled() ) {
      trace = new Exception("NSPooledConnection trace");
    }
  }

  public void unlock() {
    lastUsed = System.currentTimeMillis();
    trace = null;
  }
}


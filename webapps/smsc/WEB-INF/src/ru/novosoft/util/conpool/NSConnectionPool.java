package ru.novosoft.util.conpool;

import javax.sql.*;
import java.sql.*;
import java.util.*;

public class NSConnectionPool implements DataSource, ConnectionEventListener {
  public static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance("ru.novosoft.util.conpool");
  String dbSource;
  String dbDriver;
  String dbUser;
  String dbPassword;

  long loginTimeout = 90000; // 90 seconds
  java.io.PrintWriter logWriter = null;
  ConnectionPoolDataSource cpds;

  int maxConnections = 5;
  int minConnections = 5;
  long maxIdleTime = 86400000; // one day

  int connectionsCount = 0;
  int freeConnectionsCount = 0;

  NSPooledConnection connections[];
  NSPooledConnection freeConnections[];

  boolean shutdown = false;
  long maxShutdownTime = 600000;
  IdleWatcher idleWatcher = null;

   /**
   * Pending jobs queue tail
   */
  protected PendingJob pjobTail = null;
  /**
   * Pending jobs queue head
   */
  protected PendingJob pjobHead = null;

  public NSConnectionPool() throws SQLException {
  }

  public NSConnectionPool( Properties config, String cfgKey )  throws SQLException {
    init(config, cfgKey );
  }

  public NSConnectionPool( Properties config )  throws SQLException {
    init(config, "jdbc" );
  }

  public void init( Properties config, String cfgKey )  throws SQLException {
    dbSource = config.getProperty( cfgKey+".source" );
    if( dbSource == null ) {
      throw new SQLException( cfgKey+".source config parameter not specified" );
    }
    try {
      // test if driver already registered
      DriverManager.getDriver( dbSource );
    } catch (SQLException ex) {
      dbDriver = config.getProperty( cfgKey+".driver" );
      if( dbDriver == null ) {
        throw new SQLException( cfgKey+".driver config parameter not specified" );
      }
      try {
        Class.forName( dbDriver ).newInstance();
      } catch (Exception ex1) {
        logger.error( "", ex1 );
        throw new SQLException( "Invalid DB driver class "+dbDriver+" specified in "+cfgKey+".driver config parameter" );
      }
      try {
        DriverManager.getDriver( dbSource );
      } catch (SQLException ex1) {
        logger.error( "", ex1 );
        throw new SQLException( "Specified DB driver class "+dbDriver+" is not valid for source "+dbSource );
      }
    }
    Properties props = new Properties();
    dbUser = config.getProperty( cfgKey+".user" );
    if( dbUser == null ) {
      logger.warn( cfgKey+".user config parameter not specified" );
    } else {
      props.put( "user", dbUser );
    }
    dbPassword = config.getProperty( cfgKey+".pass" );
    if( dbPassword == null ) {
      throw new SQLException( cfgKey+".pass config parameter not specified" );
    } else {
      props.put( "password", dbPassword );
    }
    // extract extra driver parameters from jdbc.param.xxx=yyy pairs in order
    // to pass them to DriverManager.getConnection
    for (Enumeration e = config.keys(); e.hasMoreElements();) {
      String key = (String)e.nextElement();
      if( key.startsWith( "jdbc.param." ) ) {
        props.put( key.substring( "jdbc.param.".length() ), config.get( key ) );
      }
    }
    // init datasource
    cpds = new NSConnectionPoolDataSource( dbSource, props );

    String s = null;
    s = config.getProperty( cfgKey+".max.connections" );
    if( s == null ) {
    } else {
      try {
        maxConnections = Integer.valueOf( s ).intValue();
      } catch (NumberFormatException ex) {
        logger.warn( "Invalid number format for "+cfgKey+".max.connections. Using default: "+maxConnections );
      }
    }
    s = config.getProperty( cfgKey+".min.connections" );
    if( s == null ) {
    } else {
      try {
        minConnections = Integer.valueOf( s ).intValue();
      } catch (NumberFormatException ex) {
        logger.warn( "Invalid number format for "+cfgKey+".min.connections. Using default: "+minConnections );
      }
    }
    s = config.getProperty( cfgKey+".max.idle.time" );
    if( s == null ) {
    } else {
      try {
        maxIdleTime = Long.valueOf( s ).longValue()*1000;
      } catch (NumberFormatException ex) {
        logger.warn( "Invalid number format for "+cfgKey+".max.idle.time. Using default: "+(maxIdleTime/1000) );
      }
    }

    s = config.getProperty( cfgKey+".max.shutdown.time" );
    if( s == null ) {
    } else {
      try {
        maxShutdownTime = Long.valueOf( s ).longValue()*1000;
      } catch (NumberFormatException ex) {
        logger.warn( "Invalid number format for "+cfgKey+".max.shutdown.time. Using default: "+(maxShutdownTime/1000) );
      }
    }

    s = config.getProperty( cfgKey+".max.login.time" );
    if( s == null ) {
    } else {
      try {
        loginTimeout = Long.valueOf( s ).longValue()*1000;
      } catch (NumberFormatException ex) {
        logger.warn( "Invalid number format for "+cfgKey+".max.login.time. Using default: "+(loginTimeout/1000) );
      }
    }
    connections = new NSPooledConnection[maxConnections];
    freeConnections = new NSPooledConnection[maxConnections];

    // preinit connections
    synchronized (connections) {
      for (int i = 0; i < minConnections; i++) {
        NSPooledConnection pcon = (NSPooledConnection)cpds.getPooledConnection();
        if( logger.isDebugEnabled() ) logger.debug("Preinit pcon="+pcon);
        pcon.addConnectionEventListener( this );
        connections[connectionsCount++] = pcon;
        freeConnections[freeConnectionsCount++] = pcon;
      }
    }

/*    if( maxConnections > minConnections ) {
      idleWatcher = new IdleWatcher( this );
      idleWatcher.start();
    }*/
  }

  protected void finalize() throws Throwable {
    logger.debug( "Finalizing connection pool" );
    try {
      shutdown();
    } catch (SQLException ex) {
      logger.error("Finalization aborted", ex);
    }
  }

  public void shutdown() throws SQLException {
    synchronized (connections) {
      if( shutdown ) return;
      if( idleWatcher != null ) idleWatcher.shutdown();
      shutdown = true;
      while (freeConnectionsCount > 0 ) {
        NSPooledConnection pcon = freeConnections[freeConnectionsCount-1];
        if( logger.isDebugEnabled() ) logger.debug( "Shutdown: close pcon="+pcon );
        removeConnection( pcon );
        freeConnectionsCount--;
        try {
          pcon.close();
        } catch (SQLException ex) {
          logger.error("Can't close physical connection", ex);
        }
      }
      long waitStartedAt = System.currentTimeMillis();
      long lastReportedMinute = 0;
      while( connectionsCount > 0 ) {
        // waiting while other threads complete work with connections
        try {
          connections.wait( 1000 );
        } catch (InterruptedException ie) {
        }
        if( (System.currentTimeMillis() - waitStartedAt) > maxShutdownTime ) throw new SQLException( "Connection pool couldn't shutdown in "+(maxShutdownTime/1000)+" seconds. Not all connections were closed. Exiting shutdown procedure." );
        long curMinute = (System.currentTimeMillis() - waitStartedAt)/60000;
        if( curMinute > lastReportedMinute ) {
          lastReportedMinute = curMinute;
          logger.warn( "Connection pool shuting down "+lastReportedMinute+" minutes. "+connectionsCount+" unclosed connections left." );
        }
      }
    }
  }

  /**
   * Remove connection from connections list.
   * Should be called from synchronized block.
   * @param con pooled connection to remove
   */
  protected void removeConnection( NSPooledConnection pcon ) {
    for( int i = connectionsCount-1; i >= 0; i-- ) {
      if( connections[i] == pcon ) {
        if( i < connectionsCount-1 ) {
          System.arraycopy( connections, i+1, connections, i, connectionsCount - i - 1 );
        }
        --connectionsCount;
        break;
      }
    }
  }

  protected NSPooledConnection popFreeConnection() {
    if( freeConnectionsCount > 0 ) return freeConnections[--freeConnectionsCount];
    else return null;
  }

  protected void pushFreeConnection( NSPooledConnection pcon ) {
    freeConnections[freeConnectionsCount++] = pcon;
  }

  boolean lockedReport = true;

  public java.sql.Connection getConnection()  throws SQLException {
    if( shutdown ) throw new SQLException("Connection pool shut down");
    PendingJob pjob = null;
    synchronized (connections) {
      NSPooledConnection pcon = popFreeConnection();
      if( pcon != null ) {
        pcon.lock();
        lockedReport = true;
        return pcon.getConnection();
      }
      if( maxConnections > connectionsCount ) {
        // create new one
        pcon = (NSPooledConnection)cpds.getPooledConnection();
        pcon.addConnectionEventListener( this );
        connections[connectionsCount++] = pcon;
        if( logger.isDebugEnabled() ) logger.debug("create pcon="+pcon+" cnt="+connectionsCount);
        pcon.lock();
        lockedReport = true;
        return pcon.getConnection();
      } else {
        pjob = new PendingJob();
        if( logger.isDebugEnabled() ) logger.debug("new pending pjob="+pjob);
        pushJob( pjob );
      }
    }
    synchronized (pjob) {
      if( pjob.pcon != null ) {
        pjob.pcon.lock();
        if( logger.isDebugEnabled() ) logger.debug("pending job activated pjob="+pjob);
        return pjob.pcon.getConnection();
      }
      try {
        pjob.wait( loginTimeout );
      } catch (InterruptedException ie) {
      }
      if( pjob.pcon != null ) {
        if( logger.isDebugEnabled() ) logger.debug("pending job activated after wait pjob="+pjob);
        pjob.pcon.lock();
        return pjob.pcon.getConnection();
      }
    }
    // unsuccessfull
    if( shutdown ) throw new SQLException("Connection pool shut down.");
    synchronized (connections) {
      removeJob( pjob );
      if( lockedReport && logger.isDebugEnabled() ) {
        lockedReport = false;
        logger.warn( "!!!!!! Locked connections report: max="+maxConnections+" cur="+connectionsCount+" free="+freeConnectionsCount );
        for (int i = 0; i < connectionsCount; i++) {
          logger.warn( "connection #"+i+" ("+connections[i].getConnection()+") locked for "+(System.currentTimeMillis()-connections[i].lastUsed), connections[i].trace );
        }
        logger.warn( "!!!!!! Locked connections report end." );
      }
    }
    throw new SQLException("All connections locked. Timeout reached.");
  }

  public void connectionClosed(ConnectionEvent closeEvent) {
    NSPooledConnection pcon = (NSPooledConnection)closeEvent.getSource();
    if( shutdown ) {
      synchronized( connections ) {
        if( logger.isDebugEnabled() ) logger.debug("con closed because of shutdown pcon="+pcon);
        removeConnection( pcon );
      }
      try {
        pcon.close();
      } catch (SQLException ex) {
        logger.error("Can't close physical connection", ex);
      }
      return;
    }
    PendingJob pjob = null;
    synchronized (connections) {
      pjob = popJob();
      if( pjob != null ) {
        pjob.pcon = pcon;
      } else {
        pushFreeConnection(pcon);
        return;
      }
    }
    synchronized (pjob) {
      pjob.notify();
    }
  }

  public void connectionErrorOccurred(ConnectionEvent errorEvent) {
    NSPooledConnection pcon = (NSPooledConnection)errorEvent.getSource();
    logger.error( "Pooled connection error occured", errorEvent.getSQLException() );
  }
  
  /**
   * This function not implemented and will not be.
   * Just create one more connection pool with required userid and password.
   */
  public java.sql.Connection getConnection(java.lang.String A, java.lang.String B)  throws SQLException {
    throw new SQLException( "Function NSConnectionPool.getConnection( user, password) not implemented" );
  }

  public int getLoginTimeout()  throws SQLException {
    return (int)(loginTimeout/1000);
  }
  public void setLoginTimeout(int val)  throws SQLException {
    loginTimeout = ((long)val)*1000;
  }

  public java.io.PrintWriter getLogWriter()  throws SQLException {
    return logWriter;
  }

  public void setLogWriter(java.io.PrintWriter lw)  throws SQLException {
    logWriter = lw;
  }


  /**
   * This class represents waiting job in wait queue.
   */
  class PendingJob {
    /**
     * Link to next pending job in queue
     */
    public PendingJob next = null;
    /**
     * Point to connection that will handle this job
     */
    public NSPooledConnection pcon = null;

    /**
     * Constructs new job
     */
    public PendingJob() {
    }
  }

  /**
   * Remove pending job from queue.
   * @param pjob PendingJob to remove
   */
  protected void removeJob( PendingJob pjob ) {
    // find pending job
    PendingJob prevJob = null;
    PendingJob curJob = pjobHead;
    while( curJob != null ) {
      if( curJob == pjob ) {
        if( prevJob == null ) {
          // pjob at head
          pjobHead = pjob.next;
          if( pjobHead == null ) pjobTail = null;
        } else {
          prevJob.next = pjob.next;
          // check if pjob was at tail
          if( prevJob.next == null ) pjobTail = prevJob;
        }
        return;
      }
      curJob = curJob.next;
    }
  }

  /**
   * Pop pending job from queue head.
   * @return PendingJob if queue not empty, else return null
   */
  protected PendingJob popJob() {
    if( pjobHead != null ) {
      PendingJob pjob = pjobHead;
      pjobHead = pjob.next;
      if( pjobHead == null ) pjobTail = null;
      return pjob;
    } else {
      return null;
    }
  }

  /**
   * Push new pending job to queue tail
   * @param pjob pending job
   */
  protected void pushJob( PendingJob pjob ) {
    if( pjobTail != null ) {
      pjobTail.next = pjob;
      pjobTail = pjob;
    } else {
      pjobHead = pjob;
      pjobTail = pjob;
    }
  }

  protected void processIdleConnections() {
    if( connectionsCount <= minConnections ) return;
    synchronized( connections ) {
      long now = System.currentTimeMillis();
      while( connectionsCount > minConnections ) {
        if( freeConnectionsCount == 0 ) return;
        if( now-freeConnections[0].lastUsed < maxIdleTime ) {
          //  at 0 index oldest connection. No mean to check others.
          break;
        }
        NSPooledConnection pcon = freeConnections[0];
        if( logger.isDebugEnabled() ) logger.debug( "Removing idle connection "+pcon+" that was idle "+((now-pcon.lastUsed)/1000) );
        // remove connection from free list
        if( (--freeConnectionsCount) > 0 )
          System.arraycopy(freeConnections, 1, freeConnections, 0, freeConnectionsCount );
        removeConnection( pcon );
        try {
          pcon.close();
        } catch (SQLException ex) {
          logger.error("Can't close physical connection", ex);
        }
      }
    }
  }

  class IdleWatcher extends Thread {
    NSConnectionPool pool;
    boolean shutdown = false;

    public IdleWatcher(NSConnectionPool pool) {
      this.pool = pool;
    }

    public synchronized void shutdown() {
      shutdown = true;
      this.notify();
    }

    public void run() {
      while (true) {
        try {
          if( shutdown ) return;
          pool.processIdleConnections();
          // check every 10 minutes
          synchronized (this) {
            this.wait(600000);
          }
        } catch (Throwable ex) {
          logger.error("", ex);
        }
      }
    }
  }
  
}

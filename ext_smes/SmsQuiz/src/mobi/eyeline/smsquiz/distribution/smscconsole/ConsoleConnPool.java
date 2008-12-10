package mobi.eyeline.smsquiz.distribution.smscconsole;


import org.apache.log4j.Logger;

import java.util.LinkedList;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.ThreadFactory;

/**
 * author: alkhal
 */
public class ConsoleConnPool {

  private static final Logger logger = Logger.getLogger(ConsoleConnPool.class);

  private final LinkedList<ConsoleConnection> connections;

  private final int size;

  private final String login;

  private final String password;

  private final String host;

  private final long connTimeout;

  private final int port;

  private boolean shutdowned = false;

  private Lock lock = new ReentrantLock();

  private final ScheduledExecutorService closerExecutor;

  private int count = 0;


  public ConsoleConnPool(int size, String host, int port, String login,
                         String pass, long connTimeout) {
    connections = new LinkedList<ConsoleConnection>();
    this.size = size;
    this.login = login;
    this.password = pass;
    this.host = host;
    this.port = port;
    this.connTimeout = connTimeout;
    closerExecutor = Executors.newSingleThreadScheduledExecutor(new ThreadFactory(){
      public Thread newThread(Runnable r) {
        return new Thread(r,"ConnectionCloser");
      }
    });
    closerExecutor.scheduleAtFixedRate(new ConnectionCloser(), connTimeout,
        connTimeout, java.util.concurrent.TimeUnit.MILLISECONDS);
  }

  public ConsoleConnection getConnection() throws ConsoleException {
    try{
      if(shutdowned) {
        logger.error("ConnectionPool was shutdowned");
        throw new ConsoleException("ConnectionPool was shutdowned");
      }
      lock.lock();
      if(shutdowned) {
        logger.error("ConnectionPool was shutdowned");
        throw new ConsoleException("ConnectionPool was shutdowned");
      }
      ConsoleConnection conn = null;
      for(ConsoleConnection c : connections) {
        if(!c.isAssigned()) {
          conn = c; break;
        }
      }

      if(conn == null) {                      // if all connections are busy try to make one more
        if(connections.size() == size) {
          logger.error("All connections are busy");
          throw new ConsoleException("All connections are busy");
        }
        count++;
        conn =  new ConsoleConnection(Integer.toString(count),login, password, host, port);
        connections.add(conn);
      }
      conn.setAssigned(true);

      if(logger.isInfoEnabled()) {
        logger.info("Getting connection: "+conn);
      }
      return conn;

    } catch (Exception e) {
      logger.error(e,e);
      throw new ConsoleException(e);
    }finally {
      lock.unlock();
    }
  }

  public void shutdown() {
    closeAllConnections(false, false);
    closerExecutor.shutdown();
  }

  public boolean isShutdowned() {
    return shutdowned;
  }

  public int getSize() {
    return size;
  }

  private void closeAllConnections(boolean checkAssigned, boolean checkTimeout) {
    try{
      lock.lock();
      for(ConsoleConnection c : connections) {
        if(checkAssigned && c.isAssigned()) {           // if connection is assigned don't close it
         continue;
        }
        if(checkTimeout && (System.currentTimeMillis()<c.getLastUsage() + connTimeout)) {  //timeout
         continue;
        }
        if(c.isConnected()) {                           //  close connection if connected
          c.disconnect();
          if(logger.isInfoEnabled()) {
            logger.info("ConsoleCloser closed connection...");
          }
        }
      }
    } finally {
      lock.unlock();
    }
  }

  private class ConnectionCloser implements Runnable {

    public void run() {
      logger.info("ConnectionCloser starts...");
      try {
        closeAllConnections(true, true);
      } catch (Exception e) {
        logger.error(e,e);
        e.printStackTrace();
      }
      logger.info("ConnectionCloser finished");
    }
  }

}

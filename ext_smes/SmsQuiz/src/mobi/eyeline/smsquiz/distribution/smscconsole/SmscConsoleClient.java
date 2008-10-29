package mobi.eyeline.smsquiz.distribution.smscconsole;

import org.apache.log4j.Logger;

import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * author: alkhal
 */
public class SmscConsoleClient {

  private static Logger logger = Logger.getLogger(SmscConsoleClient.class);

  private CachedConsoleSender sd;
  private ScheduledExecutorService scheduledConnCloser;
  private ConnectionCloser connectionCloser;

  private long timeout;

  public SmscConsoleClient(String login, String password, String host, int port, long timeout, long closerPeriod) {
    this.timeout = timeout;
    sd = new CachedConsoleSender(login, password, host, port);
    scheduledConnCloser = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
      public Thread newThread(Runnable r) {
        return new Thread(r, "ConnectionCloser");
      }
    });
    connectionCloser = new ConnectionCloser(sd);
    scheduledConnCloser.scheduleAtFixedRate(connectionCloser, 10, closerPeriod, java.util.concurrent.TimeUnit.SECONDS);
  }

  public SmscConsoleResponse sendCommand(String command) throws SmscConsoleException {
    try {
      sd.connect();
      return sd.sendCommand(command);
    } finally {
      sd.disconnect();
    }
  }

  public void shutdown() {
    if (scheduledConnCloser != null) {
      scheduledConnCloser.shutdown();
    }
    connectionCloser.closefinally();
  }

  private class CachedConsoleSender implements ConsoleSender {

    private long lastWorkTime;
    private ConsoleSenderImpl senderImpl;
    private Lock lock;
    private boolean active = false;

    private CachedConsoleSender(String login, String password, String host, int port) {
      lock = new ReentrantLock();
      senderImpl = new ConsoleSenderImpl(login, password, host, port);
      lastWorkTime = System.currentTimeMillis();
    }

    public void connect() throws SmscConsoleException {
      lock.lock();
      logger.info("Console's locked..");
      try {
        senderImpl.connect();
        active = true;
      } catch (SmscConsoleException e) {
        lock.unlock();
        throw e;
      }
    }

    public void disconnect() {
      lock.unlock();
      logger.info("Console's unlocked");
    }

    public SmscConsoleResponse sendCommand(String command) throws SmscConsoleException {
      return senderImpl.sendCommand(command);
    }

    public long getLastWorkTime() {
      return lastWorkTime;
    }

    public void closeExt() {
      active = false;
      senderImpl.disconnect();
      logger.info("Console's closed");
      disconnect();
    }

    public boolean isActive() {
      return active;
    }
  }

  private class ConnectionCloser implements Runnable {

    private CachedConsoleSender sender;
    private boolean ignoreTimeout = false;

    public ConnectionCloser(CachedConsoleSender sender) {
      this.sender = sender;
    }

    public void closefinally() {
      ignoreTimeout = true;
      run();
    }

    public void run() {
      logger.info("ConsoleCloser starts...");
      long _timeout = timeout;
      if (ignoreTimeout) {
        _timeout = 0;
      }
      if ((System.currentTimeMillis() - sender.getLastWorkTime()) > _timeout) {
        try {
          if (sender.isActive()) {
            sender.connect();
            sender.closeExt();
            logger.info("ConsoleCloser closed a connection");
          }
        } catch (SmscConsoleException e) {
          e.printStackTrace();
        }
      }
      logger.info("ConsoleCloser finished");
    }
  }

}

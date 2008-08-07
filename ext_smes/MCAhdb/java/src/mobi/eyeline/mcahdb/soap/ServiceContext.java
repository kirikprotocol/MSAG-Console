package mobi.eyeline.mcahdb.soap;

import mobi.eyeline.mcahdb.engine.EventsFetcher;
import mobi.eyeline.mcahdb.GlobalConfig;

import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.Condition;
import java.text.SimpleDateFormat;

/**
 * User: artem
 * Date: 04.08.2008
 */

public class ServiceContext {

  private static final Lock lock = new ReentrantLock();
  private static final Condition contextInitialized = lock.newCondition();

  private static ServiceContext instance = null;

  public static ServiceContext getInstance() throws InterruptedException {
    try {
      lock.lock();

      if (instance == null)
        contextInitialized.await();

      return instance;
    } finally {
      lock.unlock();
    }
  }

  static void init(EventsFetcher fetcher, SoapConfig config) {
    try {
      lock.lock();

      if (instance == null)
        instance = new ServiceContext(fetcher, config);

      contextInitialized.signal();

    } finally {
      lock.unlock();
    }
  }

  private final EventsFetcher eventsFetcher;
  private final SimpleDateFormat df;
  private final int eventsFetchInterval;

  public ServiceContext(EventsFetcher eventsFetcher, SoapConfig config) {
    this.eventsFetcher = eventsFetcher;
    this.df = new SimpleDateFormat(config.getSoapResponseDateFormat());
    this.eventsFetchInterval = config.getSoapEventsFetchInterval();
  }

  public EventsFetcher getEventsFetcher() {
    return eventsFetcher;
  }

  public SimpleDateFormat getDf() {
    return df;
  }

  public int getEventsFetchInterval() {
    return eventsFetchInterval;
  }
}

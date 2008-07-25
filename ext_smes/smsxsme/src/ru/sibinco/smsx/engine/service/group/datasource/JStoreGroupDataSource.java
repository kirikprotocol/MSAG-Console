package ru.sibinco.smsx.engine.service.group.datasource;

import ru.sibinco.smsx.utils.DataSourceException;
import com.eyeline.jstore.JStore;
import com.eyeline.utils.XDelayedQueue;

import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import org.apache.log4j.Category;

/**
 * User: artem
 * Date: 16.07.2008
 */

public class JStoreGroupDataSource implements GroupDataSource {

  private static final Category log = Category.getInstance(JStoreGroupDataSource.class);

  private final Lock lock = new ReentrantLock();

  private final JStore store;
  private final XDelayedQueue<Long, Long> queue;
  private final int dataLifeTime;
  private final StoreCleanThread cleaner;

  public JStoreGroupDataSource(String storeFile, int dataLifeTime) {
    this.store = new JStore(-1);
    this.store.init(storeFile, 10000, 1000);
    this.queue = new XDelayedQueue<Long, Long>();
    this.dataLifeTime = dataLifeTime;
    this.cleaner = new StoreCleanThread();
    this.cleaner.start();

    store.first();
    long val;
    while((val = store.next()) != -1)
      queue.offer(val, val, System.currentTimeMillis() + dataLifeTime);    
  }

  public void setStatus(long msgKey, MessageStatus status) throws DataSourceException {
    try {
      lock.lock();

      store.put(msgKey, status.getId());

      queue.offer(msgKey, msgKey, System.currentTimeMillis() + dataLifeTime);

    } finally {
      lock.unlock();
    }
  }

  public boolean containsStatus(long msgKey) throws DataSourceException {
    return store.get(msgKey) != -1;
  }


  public MessageStatus getStatus(long msgKey) throws DataSourceException {
    try {
      lock.lock();
      return MessageStatus.getById(store.get(msgKey));
    } finally {
      lock.unlock();
    }
  }

  public void shutdown() {
    try {
      lock.lock();

      store.shutdown();
      cleaner.shutdown();
    } finally {
      lock.unlock();
    }
  }

  private class StoreCleanThread extends Thread {
    private volatile boolean started = true;

    public StoreCleanThread() {
      super("JStoreGroupDataSourceCleaner");
    }

    public void shutdown() {
      started = false;
    }

    public void run() {
      while(started) {
        try {
          Long key = queue.poll(10000);
          if (key == null)
            continue;

          try {
            lock.lock();

            store.remove(key);

          } finally {
            lock.unlock();
          }
        } catch (InterruptedException e) {
          log.error(e,e);
        }
      }
    }
  }
}

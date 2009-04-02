package ru.sibinco.smsx.engine.service.group.datasource;

import com.eyeline.jstore.JStore;
import com.eyeline.utils.FixedArrayCache;
import ru.sibinco.smsx.utils.DataSourceException;

import java.io.File;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * User: artem
 * Date: 02.12.2008
 */
public class RepliesMap {

  private final JStore store;
  private final FixedArrayCache<Long> cache;
  private final Lock lock = new ReentrantLock();

  public RepliesMap(File file, int cacheSize) {
    store = new JStore(-1);
    store.init(file.getAbsolutePath(), 60000, 10);
    cache = new FixedArrayCache<Long>(cacheSize);

    // load cache
    int i = 0;
    for (long msisdn : store.keySet()) {
      if (i < cacheSize)
        cache.add(msisdn);
      else
        store.remove(msisdn);
      i++;
    }
  }

  public void put(String msisdn, int listId) throws DataSourceException {
    long msisdnLong = getKey(msisdn);
    try {
      lock.lock();
      int key;
      if ((key = cache.contains(msisdnLong)) >= 0) {
        cache.update(key);
      } else {
        if (cache.isFull()) {
          long lastMsisdn = cache.getLast();
          store.remove(lastMsisdn);
        }
        cache.add(msisdnLong);
      }

      store.put(msisdnLong, listId);
    } finally {
      lock.unlock();
    }
  }

  private static long getKey(String msisdn) {
    if (msisdn.charAt(0) == '+')
      return Long.parseLong(msisdn.substring(1));
    return Long.parseLong(msisdn);
  }

  public int get(String msisdn) {
    try {
      lock.lock();
      return store.get(getKey(msisdn));
    } finally {
      lock.unlock();
    }
  }

  public int size() {
    try {
      lock.lock();
      return store.size();
    } finally {
      lock.unlock();
    }
  }

  public void shutdown() {
    try {
      lock.lock();
      store.shutdown();
    } finally {
      lock.unlock();
    }
  }


  public static void main(String args[]) throws DataSourceException {
    RepliesMap m = new RepliesMap(new File("replies.bin"), 3);
    m.put("+79139023974", 1);
    m.put("+79139023975", 2);
    m.put("+79139023976", 13);
    m.put("+79139023977", 14);
    m.put("+79139023978", 14);
    m.put("+79139023979", 14);

    System.out.println(m.size());
  }
}

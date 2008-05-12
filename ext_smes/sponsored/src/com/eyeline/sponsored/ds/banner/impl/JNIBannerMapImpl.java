package com.eyeline.sponsored.ds.banner.impl;

import com.eyeline.jstore.JStore4Java;
import com.eyeline.sponsored.ds.banner.BannerMap;
import com.eyeline.sponsored.utils.CalendarUtils;
import org.apache.log4j.Category;

import java.io.*;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import static com.eyeline.utils.IOUtils.*;

/**
 * User: artem
 * Date: 11.04.2008
 */

public final class JNIBannerMapImpl implements BannerMap {

  private static final Category log = Category.getInstance("DS");

  private final String keyStoreFile;
  private final JStore4Java store;
  private final ScheduledExecutorService mapCleaner;
  private final Lock lock = new ReentrantLock();

  private volatile long lastKey = -1;


  public JNIBannerMapImpl(String storeFile, int rollingTime, int maxCollizions, String keyStoreFile) {
    this.store = new JStore4Java();
    this.keyStoreFile = keyStoreFile;
    this.store.Init(storeFile, rollingTime, maxCollizions);

    System.out.println("jstore size=" + size());

    this.mapCleaner = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
      public Thread newThread(Runnable r) {
        return new Thread(r, "JNIBannerMapCleaner");
      }
    });

    this.mapCleaner.scheduleAtFixedRate(new KeyLogTask(),
                                        CalendarUtils.getNextDayStartInMillis(new Date()) - System.currentTimeMillis(),
                                        24 * 3600 * 1000,
                                        TimeUnit.MILLISECONDS);

  }

  public void put(long messageId, int advertiserId) {
    try {
      lock.lock();
      store.Insert(messageId, advertiserId);
      lastKey = messageId;
    } finally {
      lock.unlock();
    }
  }

  public int get(long messageId) {
    int val[] = new int[1];
    try {
      lock.lock();

      if (store.Lookup(messageId, val)) {
        store.Delete(messageId);
        return val[0];
      }

      return Integer.MIN_VALUE;
    } finally {
      lock.unlock();
    }

  }

  public void close() {
    mapCleaner.shutdown();
  }

  public int size() {
    long key[] = new long[1];

    int size = 0;

    try {
      lock.lock();

      store.First();

      while(store.Next(key)) {
        size++;
      }

    } finally {
      lock.unlock();
    }

    return size;
  }

  public void clear(long minMessageId) {
    long key[] = new long[1];
    final List<Long> buffer = new ArrayList<Long>(1000);

    try {
      lock.lock();

      store.First();

      while(store.Next(key)) {
        if (key[0] < minMessageId)
          buffer.add(key[0]);
      }

      for (long k : buffer)
        store.Delete(k);

    } finally {
      lock.unlock();
    }
  }


  private class KeyLogTask implements Runnable {

    public void run() {

      if (log.isInfoEnabled())
        log.info("Banner map clean started.");
      final ArrayList<Long> keys = new ArrayList<Long>(10);

      try {
        // Read keys
        if (log.isInfoEnabled())
          log.info("Read keys.");
        InputStream is = null;
        try {
          is = new BufferedInputStream(new FileInputStream(keyStoreFile));

          while(true)
            keys.add(readLong(is));

        } catch (FileNotFoundException e) {
        } catch (EOFException e) {
        } catch (IOException e) {
          log.error(e,e);
        } finally {
          try {
            if (is != null)
              is.close();
          } catch (IOException e) {
          }
        }

        try {
          lock.lock();
          // Clear map
          if (keys.size() >= 3) {
            long key = keys.get(0);
            if (log.isInfoEnabled())
              log.info("Remove keys less than " + key);
            clear(key);
            // Remove first key and add last
            keys.remove(0);
          }

          if (log.isInfoEnabled())
            log.info("Store lask key " + lastKey);

          keys.add(lastKey);
        } finally {
          lock.unlock();
        }

        // Store keys
        if (log.isInfoEnabled())
          log.info("Store keys.");
        OutputStream os = null;
        try {
          os = new BufferedOutputStream(new FileOutputStream(keyStoreFile, false));

          for (long key : keys)
            writeLong(key, os);

        } catch (IOException e) {
          log.error(e,e);
        } finally {
          try {
            if (os != null)
              os.close();
          } catch (IOException e) {
          }
        }

        if (log.isInfoEnabled())
          log.info("Banner map clean finished.");

      } catch (Throwable e) {
        log.error(e,e);
      }
    }
  }


  public static void main(String[] args) {

    final JNIBannerMapImpl map = new JNIBannerMapImpl("store/banner.bin", 500, 10000, "store/key_log.bin");
    Object o = new Object();

    int i=10000000;
    for (int k=0; k<10; k++) {
      System.out.println("fill");
      for (int j=0; j<1000000; j++) {
        i++;
        map.put(i,j);
      }
      System.out.println("lookup");
      for (int j=0; j<1000000; j++) {
        int val = map.get(i-j);
        if (val == Integer.MIN_VALUE) {
          System.out.println("Can't find " + (i-j));
          return;
        }
      }
    }
    System.out.println("size = " + map.size());
  }

}

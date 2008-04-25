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

    public int readByte(InputStream is) throws IOException {
      int b = is.read();
      if (b == -1)
        throw new EOFException();
      return b;
    }

    public long readLong(InputStream is) throws IOException {
      long ch1 = readByte(is);
      long ch2 = readByte(is);
      long ch3 = readByte(is);
      long ch4 = readByte(is);
      long ch5 = readByte(is);
      long ch6 = readByte(is);
      long ch7 = readByte(is);
      long ch8 = readByte(is);
      return (
              ((ch1 << 56) & 0xFF00000000000000L) |
              ((ch2 << 48) & 0x00FF000000000000L) |
              ((ch3 << 40) & 0x0000FF0000000000L) |
              ((ch4 << 32) & 0x000000FF00000000L) |
              ((ch5 << 24) & 0x00000000FF000000L) |
              ((ch6 << 16) & 0x0000000000FF0000L) |
              ((ch7 << 8)  & 0x000000000000FF00L) |
              (ch8         & 0x00000000000000FFL)
      );
    }

    public void writeLong(long value, OutputStream os) throws IOException {
      os.write((int)((value >> 56) & 0xFF));
      os.write((int)((value >> 48) & 0xFF));
      os.write((int)((value >> 40) & 0xFF));
      os.write((int)((value >> 32) & 0xFF));
      os.write((int)((value >> 24) & 0xFF));
      os.write((int)((value >> 16) & 0xFF));
      os.write((int)((value >> 8) & 0xFF));
      os.write((int)((value) & 0xFF));
    }

    public void run() {

      final ArrayList<Long> keys = new ArrayList<Long>(10);

      try {
        // Read keys
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
            clear(keys.get(0));
            // Remove first key and add last
            keys.remove(0);
          }

          keys.add(lastKey);
        } finally {
          lock.unlock();
        }

        // Store keys
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

      } catch (Throwable e) {
        log.error(e,e);
      }
    }
  }


  public static void main(String[] args) {

    final JNIBannerMapImpl map = new JNIBannerMapImpl("store/banner.bin", 500, 10000, "store/key_log.bin");
    Object o = new Object();

    for (int j=0; j<10; j++) {
      System.out.println("fill");

      for (int i=0; i<10000; i++) {
        map.put(i, 0);
        synchronized(o) {
          try {
            o.wait(100);
          } catch (InterruptedException e) {
            e.printStackTrace();
          }
        }
        if (i % 100 == 0)
          System.out.println("size = " + map.size());
      }

//      System.out.println("sizes");
//      for (int i=0; i<10000; i++) {
//        synchronized(o) {
//          try {
//            o.wait(1000);
//          } catch (InterruptedException e) {
//            e.printStackTrace();
//          }
//          System.out.println(map.size());
//        }
//      }
    }

    System.out.println("size = " + map.size());
  }

}

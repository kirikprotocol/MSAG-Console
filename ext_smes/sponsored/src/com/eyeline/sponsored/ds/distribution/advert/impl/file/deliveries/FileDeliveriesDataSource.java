package com.eyeline.sponsored.ds.distribution.advert.impl.file.deliveries;

import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.DataSourceTransaction;
import com.eyeline.sponsored.ds.distribution.advert.DeliveriesDataSource;
import com.eyeline.sponsored.ds.distribution.advert.Delivery;
import com.eyeline.sponsored.utils.CalendarUtils;
import org.apache.log4j.Category;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * User: artem
 * Date: 28.03.2008
 */

public class FileDeliveriesDataSource implements DeliveriesDataSource {

  private static final Category log = Category.getInstance(FileDeliveriesDataSource.class);

  private static final int CACHE_CLEAN_INTERVAL = 3600000;
  private static final SimpleDateFormat df = new SimpleDateFormat("yyyyMMdd");

  private final String storeDir;

  private final HashMap<String, HashedDeliveriesFile> cache = new HashMap<String, HashedDeliveriesFile>(50);
  private final ScheduledExecutorService cleaner;
  private final Lock cacheLock = new ReentrantLock();

  public FileDeliveriesDataSource(String storeDir) {
    this.storeDir = storeDir;

    this.cleaner = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
      public Thread newThread(Runnable r) {
        return new Thread(r, "DeliveriesFilesCacheCleaner");
      }
    });

    this.cleaner.scheduleWithFixedDelay(new Runnable() {
      public void run() {
        clearCache(false);
      }
    }, CACHE_CLEAN_INTERVAL, CACHE_CLEAN_INTERVAL, TimeUnit.MILLISECONDS);
  }

  private void clearCache(boolean all) {
    try {
      cacheLock.lock();
      long now = System.currentTimeMillis();

      Map.Entry<String, HashedDeliveriesFile> e;
      for (Iterator<Map.Entry<String, HashedDeliveriesFile>> entries = cache.entrySet().iterator(); entries.hasNext();) {
        e = entries.next();

        if (all || ((now - e.getValue().getTime()) > CACHE_CLEAN_INTERVAL) && !e.getValue().isOpened()) {
          try {
            e.getValue().impl.close();
          } catch (DeliveriesFileException e1) {
            log.error(e1,e1);
          }

          entries.remove();
        }
      }

    } finally {
      cacheLock.unlock();
    }
  }

  private HashedDeliveriesFile getAndCacheFile(Date date, boolean create) throws DataSourceException {
    final String id = df.format(date) + ".deliveries";
    try {
      cacheLock.lock();

      HashedDeliveriesFile page = cache.get(id);
      if (page == null) {
        File f = new File(storeDir, id);
        if (!create && !f.exists())
          return null;
        page = new HashedDeliveriesFile(new DeliveriesFileImpl(this, f, CalendarUtils.getDayStartInMillis(date)));
        cache.put(id, page);
      }

      page.open();
      return page;
    } catch (DeliveriesFileException e) {
      throw new DataSourceException(e);
    } finally {
      cacheLock.unlock();
    }
  }

  public DataSourceTransaction createTransaction(Date date, String distrName, int volume, TimeZone tz, int size) throws DataSourceException {
    HashedDeliveriesFile f = null;
    try {
      f = getAndCacheFile(date, true);
      return f.createTransaction(distrName, volume, tz, size);
    } catch (DeliveriesFileException e) {
      throw new DataSourceException(e);
    } finally {
      if (f != null)
        f.close();
    }
  }

  public Delivery createDelivery() {
    return new DeliveryImpl(this);
  }

  void saveDelivery(DeliveryImpl d, DataSourceTransaction t) throws DataSourceException {
    HashedDeliveriesFile f = null;
    try {
      ((DeliveriesFileTransaction)t).saveDelivery(d);
    } finally {
      if (f != null)
        f.close();
    }
  }

  void updateDelivery(DeliveryImpl d) throws DataSourceException {
    HashedDeliveriesFile f = null;
    try {
      f = getAndCacheFile(d.getStartDate(), true);
      f.updateDelivery(d);
    } catch (DeliveriesFileException e) {
      throw new DataSourceException(e);
    } finally {
      if (f != null)
        f.close();
    }
  }

  public void lookupDeliveries(Date end, int limit, Collection<Delivery> result) throws DataSourceException {
    try {
      HashedDeliveriesFile f = getAndCacheFile(end, false);
      if (f != null) {
        try {
          f.open();
          f.readDeliveries(end,limit, result);
        } finally {
          f.close();
        }
      }

    } catch (DeliveriesFileException e) {
      throw new DataSourceException(e);
    }
  }

  public void lookupDeliveries(Date start, Date end, Collection<Delivery> result) throws DataSourceException {
    try {
      HashedDeliveriesFile f = getAndCacheFile(end, false);
      if (f != null) {
        try {
          f.open();
          f.readDeliveries(start, end, result);
        } finally {
          f.close();
        }
      }

    } catch (DeliveriesFileException e) {
      throw new DataSourceException(e);
    }
  }

  /**
   *
   * @param date
   * @param tz
   * @param distrName
   * @return
   * @throws DataSourceException
   */
  public int getDeliveriesCount(Date date, TimeZone tz, String distrName) throws DataSourceException {
    HashedDeliveriesFile f = null;
    try {
      int count = 0;

      // Get current date
      f = getAndCacheFile(date, false);
      if (f != null)
        count += f.getDeliveriesCount(date, tz, distrName);

      // Get previous date
      f = getAndCacheFile(CalendarUtils.getPrevDayStart(date), false);
      if (f != null)
        count += f.getDeliveriesCount(date, tz, distrName);

      return count;
    } catch (DeliveriesFileException e) {
      throw new DataSourceException(e);
    } finally {
      if (f != null)
        f.close();
    }
  }

  public void shutdown() {
    cleaner.shutdown();
    clearCache(true);
  }



















  private static final class HashedDeliveriesFile implements DeliveriesFile {
    private final DeliveriesFileImpl impl;
    private final Lock lock = new ReentrantLock();

    private long time;
    private boolean opened;

    public HashedDeliveriesFile(DeliveriesFileImpl f) {
      this.impl = f;
      this.time = System.currentTimeMillis();
      this.opened = false;
      open();
    }

    public void updateDelivery(DeliveryImpl d) throws DeliveriesFileException {
      try {
        lock.lock();
        time = System.currentTimeMillis();
        impl.updateDelivery(d);
      } finally {
        lock.unlock();
      }
    }

    public DeliveriesFileTransaction createTransaction(String distrName, int volume, TimeZone tz, int size) throws DeliveriesFileException {
      try {
        lock.lock();
        time = System.currentTimeMillis();
        return new HashedFileTransaction(impl.createTransaction(distrName, volume, tz, size));
      } finally {
        lock.unlock();
      }
    }

    public void readDeliveries(final Date startDate, final Date endDate, Collection<Delivery> result) throws DeliveriesFileException {
      try {
        lock.lock();
        time = System.currentTimeMillis();
        impl.readDeliveries(startDate, endDate, result);
      } finally {
        lock.unlock();
      }
    }

    public int getDeliveriesCount(Date date, TimeZone tz, String distrName) throws DeliveriesFileException {
      try {
        lock.lock();
        time = System.currentTimeMillis();
        return impl.getDeliveriesCount(date, tz, distrName);
      } finally {
        lock.unlock();
      }
    }

    public void readDeliveries(final Date date, final int limit, Collection<Delivery> result) throws DeliveriesFileException {
      try {
        lock.lock();
        time = System.currentTimeMillis();
        impl.readDeliveries(date, limit, result);
      } finally {
        lock.unlock();
      }
    }
    
    public void open() {
      try {
        lock.lock();
        opened = true;
        time = System.currentTimeMillis();
      } finally {
        lock.unlock();
      }
    }

    public void close() {
      try {
        lock.lock();
        opened = false;
        time = System.currentTimeMillis();
      } finally {
        lock.unlock();
      }
    }

    public boolean isOpened() {
      try {
        lock.lock();
        return opened;
      } finally {
        lock.unlock();
      }
    }

    public long getTime() {
      try {
        lock.lock();
        return time;
      } finally {
        lock.unlock();
      }
    }


    private class HashedFileTransaction implements DeliveriesFileTransaction {

      private final DeliveriesFileTransaction impl;

      public HashedFileTransaction(DeliveriesFileTransaction impl) {
        this.impl = impl;
      }

      public void saveDelivery(DeliveryImpl d) throws DataSourceException {
        try {
          lock.lock();
          time = System.currentTimeMillis();
          impl.saveDelivery(d);
        } finally {
          lock.unlock();
        }
      }

      public void commit() throws DataSourceException {
        try {
          lock.lock();
          time = System.currentTimeMillis();
          impl.commit();
        } finally {
          lock.unlock();
        }
      }

      public void rollback() throws DataSourceException {
        try {
          lock.lock();
          time = System.currentTimeMillis();
          impl.rollback();
        } finally {
          lock.unlock();
        }
      }

      public void close() {
        try {
          lock.lock();
          time = System.currentTimeMillis();
          impl.close();
        } finally {
          lock.unlock();
        }
      }
    }
  }



  public static void main(String[] args) {
    FileDeliveriesDataSource impl = new FileDeliveriesDataSource("store");
    try {
      Calendar c = Calendar.getInstance();
      c.setTimeInMillis(System.currentTimeMillis());
      c.set(Calendar.HOUR_OF_DAY, 18);
      c.set(Calendar.MINUTE, 54);
      c.set(Calendar.SECOND, 0);
      c.set(Calendar.MILLISECOND, 0);

      Date d;
      for (int i=0; i<10000; i++) {
        d = c.getTime();
        c.set(Calendar.MINUTE, c.get(Calendar.MINUTE) + 1);
        long start = System.currentTimeMillis();
        Collection<Delivery> deliveries = new ArrayBlockingQueue<Delivery>(100000);
        impl.lookupDeliveries(d, c.getTime(), deliveries);
        System.out.println((System.currentTimeMillis() - start) + " : " + deliveries.size());
      }

    } catch (DataSourceException e) {
      e.printStackTrace();
    } finally {
      impl.shutdown();
    }
  }
}
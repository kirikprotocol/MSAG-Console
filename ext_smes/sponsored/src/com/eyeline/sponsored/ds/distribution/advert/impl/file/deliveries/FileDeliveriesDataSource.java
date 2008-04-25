package com.eyeline.sponsored.ds.distribution.advert.impl.file.deliveries;

import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.DataSourceTransaction;
import com.eyeline.sponsored.ds.distribution.advert.DeliveriesDataSource;
import com.eyeline.sponsored.ds.distribution.advert.Delivery;
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
          } catch (DataSourceException e1) {
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
        page = new HashedDeliveriesFile(new DeliveriesFileImpl(f));
        cache.put(id, page);
      }

      page.open();
      return page;
    } finally {
      cacheLock.unlock();
    }
  }

  public DataSourceTransaction createInsertTransaction(Date startDate, Date endDate, String distrName, int volume, TimeZone tz, int size) throws DataSourceException {
    HashedDeliveriesFile f = null;
    HashedDeliveriesFile f1 = null;
    try {
      f = getAndCacheFile(startDate, true);
      f.open();

      f1 = getAndCacheFile(endDate, true);
      f1.open();

      if (f1 == f)
        return f.createInsertTransaction(startDate, endDate, distrName, volume, tz, size);
      else
        return new MultiFileTransaction(new DeliveriesFileTransaction[] {f.createInsertTransaction(startDate, endDate, distrName, volume, tz, size),
                                                                         f1.createInsertTransaction(startDate, endDate, distrName, volume, tz, size)});

    } finally {
      if (f != null)
        f.close();

      if (f1 != null && f1 != f)
        f1.close();
    }
  }

  public Delivery createDelivery() {
    return new DeliveryImpl();
  }

  public void lookupDeliveries(Date end, int limit, Collection<Delivery> result) throws DataSourceException {
    HashedDeliveriesFile f = null;
    try {
      f = getAndCacheFile(end, false);
      if (f != null) {
        f.open();
        f.lookupDeliveries(end,limit, result);
      }
    } finally {
      if (f != null)
        f.close();
    }
  }

  public void lookupDeliveries(Date start, Date end, Collection<Delivery> result) throws DataSourceException {
    HashedDeliveriesFile f = null;
    try {
      f = getAndCacheFile(end, false);
      if (f != null) {
        f.open();
        f.lookupDeliveries(start, end, result);
      }
    } finally {
      if (f != null)
        f.close();
    }
  }

  public boolean hasDeliveries(Date date, TimeZone tz, String distrName) throws DataSourceException {
    HashedDeliveriesFile f = null;
    try {

      // Get current date
      f = getAndCacheFile(date, false);
      if (f != null) {
        f.open();
        return f.hasDeliveries(date, tz, distrName);
      }

      return false;
    } catch (DataSourceException e) {
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

    private volatile long time;
    private volatile int transactions;
    private volatile boolean opened;

    public HashedDeliveriesFile(DeliveriesFileImpl f) {
      this.impl = f;
      this.time = System.currentTimeMillis();
      this.opened = false;
      open();
    }

    public DeliveriesFileTransaction createInsertTransaction(Date startDate, Date endDate, String distrName, int volume, TimeZone tz, int size) throws DataSourceException {
      transactions++;
      return new HashedFileTransaction(this, impl.createInsertTransaction(startDate, endDate, distrName, volume, tz, size));
    }

    public void lookupDeliveries(final Date startDate, final Date endDate, Collection<Delivery> result) throws DataSourceException {
      impl.lookupDeliveries(startDate, endDate, result);
    }

    public boolean hasDeliveries(Date date, TimeZone tz, String distrName) throws DataSourceException {
      return impl.hasDeliveries(date, tz, distrName);
    }

    public void lookupDeliveries(final Date date, final int limit, Collection<Delivery> result) throws DataSourceException {
      impl.lookupDeliveries(date, limit, result);
    }
    
    public void open() {
      opened = true;
      time = System.currentTimeMillis();
    }

    public void close() {
      opened = false;
      time = System.currentTimeMillis();
    }

    public boolean isOpened() {
      return opened || transactions > 0;
    }

    public void closeTransaction() {
      transactions--;
      time = System.currentTimeMillis();
    }

    public long getTime() {
      return time;
    }
  }



  private static class HashedFileTransaction implements DeliveriesFileTransaction {

    private final DeliveriesFileTransaction impl;
    private final HashedDeliveriesFile file;

    public HashedFileTransaction(HashedDeliveriesFile file, DeliveriesFileTransaction impl) {
      this.impl = impl;
      this.file = file;
    }

    public void saveDelivery(DeliveryImpl d) throws DataSourceException {
      impl.saveDelivery(d);
    }

    public void commit() throws DataSourceException {
      impl.commit();
    }

    public void rollback() throws DataSourceException {
      impl.rollback();
    }

    public void close() {
      impl.close();
      file.closeTransaction();
    }
  }



  private static class MultiFileTransaction implements DeliveriesFileTransaction {

    private final DeliveriesFileTransaction[] transactions;

    public MultiFileTransaction(DeliveriesFileTransaction[] transactions) {
      this.transactions = transactions;
    }

    public void saveDelivery(DeliveryImpl d) throws DataSourceException {
      for (DeliveriesFileTransaction tx : transactions)
        tx.saveDelivery(d);
    }

    public void commit() throws DataSourceException {
      for (DeliveriesFileTransaction tx : transactions)
        tx.commit();
    }

    public void rollback() throws DataSourceException {
      for (DeliveriesFileTransaction tx : transactions)
        tx.rollback();
    }

    public void close() {
      for (DeliveriesFileTransaction tx : transactions)
        tx.close();
    }
  }



  public static void main(String[] args) {
    FileDeliveriesDataSource impl = new FileDeliveriesDataSource("store");
    try {
      Calendar c = Calendar.getInstance();
      c.setTimeInMillis(System.currentTimeMillis());
      c.set(Calendar.HOUR_OF_DAY, 17);
      c.set(Calendar.MINUTE, 37);
      c.set(Calendar.SECOND, 0);
      c.set(Calendar.MILLISECOND, 0);

      Date d;
      long total = 0;
      for (;;) {
        d = c.getTime();
        c.set(Calendar.SECOND, c.get(Calendar.SECOND) + 10);
        long start = System.currentTimeMillis();
        Collection<Delivery> deliveries = new ArrayBlockingQueue<Delivery>(100000);
        impl.lookupDeliveries(d, c.getTime(), deliveries);
        if (deliveries.isEmpty())
          break;
        total += deliveries.size();
//        System.out.println((System.currentTimeMillis() - start) + " : " + deliveries.size());
      }
      System.out.println("total = " + total);

    } catch (DataSourceException e) {
      e.printStackTrace();
    } finally {
      impl.shutdown();
    }
  }
}
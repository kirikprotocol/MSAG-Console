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
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;
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

  private HashedDeliveriesFile getFile(Date date, boolean create) throws DataSourceException {
    final String id = df.format(date) + ".deliveries";
    try {
      cacheLock.lock();

      HashedDeliveriesFile page = cache.get(id);
      if (page == null) {
        File f = new File(storeDir, id);
        if (!create && !f.exists())
          return null;
        page = new HashedDeliveriesFile(new DeliveriesFile(this, f, CalendarUtils.getDayStartInMillis(date)));
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

  public DataSourceTransaction createTransaction() throws DataSourceException {
    return new FakeTransaction();
  }

  public Delivery createDelivery() {
    return new DeliveryImpl(this);
  }

  void saveDelivery(DeliveryImpl d) throws DataSourceException {
    HashedDeliveriesFile f = null;
    try {
      f = getFile(d.getStartDate(), true);
      f.saveDelivery(d);
    } catch (DeliveriesFileException e) {
      throw new DataSourceException(e);
    } finally {
      if (f != null)
        f.close();
    }
  }

  void updateDelivery(DeliveryImpl d) throws DataSourceException {
    HashedDeliveriesFile f = null;
    try {
      f = getFile(d.getStartDate(), true);
      f.saveDelivery(d);
    } catch (DeliveriesFileException e) {
      throw new DataSourceException(e);
    } finally {
      if (f != null)
        f.close();
    }
  }

  public List<Delivery> lookupActiveDeliveries(Date end, int limit) throws DataSourceException {
    try {
      final LinkedList<Delivery> result = new LinkedList<Delivery>();

      HashedDeliveriesFile f = getFile(end, false);
      if (f != null) {
        try {
          f.open();
          result.addAll(f.readDeliveries(end,limit));
        } finally {
          f.close();
        }
      }

      return result;
    } catch (DeliveriesFileException e) {
      throw new DataSourceException(e);
    }
  }

  public List<Delivery> lookupActiveDeliveries(Date start, Date end) throws DataSourceException {
    try {
      final LinkedList<Delivery> result = new LinkedList<Delivery>();

      HashedDeliveriesFile f = getFile(end, false);
      if (f != null) {
        try {
          f.open();
          result.addAll(f.readDeliveries(start, end));
        } finally {
          f.close();
        }
      }

      return result;
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
      f = getFile(date, false);
      if (f != null)
        count += f.getDeliveriesCount(date, tz, distrName);

      // Get previous date
      f = getFile(CalendarUtils.getPrevDayStart(date), false);
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

  private static class FakeTransaction implements DataSourceTransaction {

    public void commit() throws DataSourceException {
    }

    public void rollback() throws DataSourceException {
    }

    public void close() {
    }
  }

  private static final class HashedDeliveriesFile {
    private final DeliveriesFile impl;

    private long time;
    private boolean opened;

    public HashedDeliveriesFile(DeliveriesFile f) {
      this.impl = f;
      this.time = System.currentTimeMillis();
      this.opened = false;
      open();
    }

    public void saveDelivery(DeliveryImpl delivery) throws DeliveriesFileException {
      time = System.currentTimeMillis();
      impl.saveDelivery(delivery);
    }

    public List<Delivery> readDeliveries(final Date startDate, final Date endDate) throws DeliveriesFileException {
      time = System.currentTimeMillis();
      return impl.readDeliveries(startDate, endDate);
    }

    public int getDeliveriesCount(Date date, TimeZone tz, String distrName) throws DeliveriesFileException {
      time = System.currentTimeMillis();
      return impl.getDeliveriesCount(date, tz, distrName);
    }

    public List<Delivery> readDeliveries(final Date date, final int limit) throws DeliveriesFileException {
      time = System.currentTimeMillis();
      return impl.readDeliveries(date, limit);
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
      return opened;
    }

    public long getTime() {
      return time;
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
        List<Delivery> deliveries = impl.lookupActiveDeliveries(d, c.getTime());
         System.out.println((System.currentTimeMillis() - start) + " : " + deliveries.size());
      }

    } catch (DataSourceException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
    } finally {
      impl.shutdown();
    }
  }
}
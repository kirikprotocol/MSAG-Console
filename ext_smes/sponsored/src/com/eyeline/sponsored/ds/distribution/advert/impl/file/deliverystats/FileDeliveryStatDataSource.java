package com.eyeline.sponsored.ds.distribution.advert.impl.file.deliverystats;

import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.ResultSet;
import com.eyeline.sponsored.ds.distribution.advert.DeliveryStat;
import com.eyeline.sponsored.ds.distribution.advert.DeliveryStatsDataSource;
import com.eyeline.sponsored.ds.distribution.advert.DeliveryStatsQuery;
import org.apache.log4j.Category;

import java.io.*;
import java.nio.channels.FileChannel;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * User: artem
 * Date: 24.03.2008
 */

public class FileDeliveryStatDataSource implements DeliveryStatsDataSource {

  private static final Category log = Category.getInstance(FileDeliveryStatDataSource.class);

  private static final int CACHE_CLEAN_INTERVAL = 3600000;
  private static final SimpleDateFormat df = new SimpleDateFormat("yyyyMMdd");

  private final String storeDir;

  private final HashMap<String, HashedStatsFile> cache = new HashMap<String, HashedStatsFile>(10);
  private final ScheduledExecutorService cleaner;
  private final ExecutorService compressor;
  private final Lock cacheLock = new ReentrantLock();

  public FileDeliveryStatDataSource(String storeDir) {
    this.storeDir = storeDir;

    this.cleaner = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
      public Thread newThread(Runnable r) {
        return new Thread(r, "StatsFilesCacheCleaner");
      }
    });

    this.compressor = Executors.newSingleThreadExecutor(new ThreadFactory() {
      public Thread newThread(Runnable r) {
        return new Thread(r, "StatsFilesCompressor");
      }
    });

    this.cleaner.scheduleWithFixedDelay(new Runnable() {
      public void run() {
        clearCache(false, true);
      }
    }, CACHE_CLEAN_INTERVAL, CACHE_CLEAN_INTERVAL, TimeUnit.MILLISECONDS);
  }

  private void clearCache(boolean all, boolean compress) {
    try {
      cacheLock.lock();
      long now = System.currentTimeMillis();

      Map.Entry<String, HashedStatsFile> e;
      for (Iterator<Map.Entry<String, HashedStatsFile>> entries = cache.entrySet().iterator(); entries.hasNext();) {
        e = entries.next();

        if (all || ((now - e.getValue().getTime()) > CACHE_CLEAN_INTERVAL) && !e.getValue().isOpened()) {

          try {
            e.getValue().impl.close();
          } catch (StatsFileException e1) {
            log.error(e1,e1);
          }

          if (compress && e.getValue().compressible)
            compressor.submit(new FileCompressTask(e.getValue(), e.getKey()));
          else
            entries.remove();
        }
      }

    } finally {
      cacheLock.unlock();
    }
  }

  private StatsFile getFile(String id, boolean create, boolean compressible) {
    try {
      cacheLock.lock();

      HashedStatsFile page = cache.get(id);
      if (page == null) {
        File f = new File(storeDir, id + ".stats");
        if (!create && !f.exists())
          return null;
        page = new HashedStatsFile(new StatsFileImpl(f), compressible);
        cache.put(id, page);
      }

      page.open();
      return page;
    } catch (IOException e) {
      return null;
    } finally {
      cacheLock.unlock();
    }
  }

  private StatsFile getFile(Date date, boolean create) {
    return getFile(df.format(date), create, true);
  }

  private  ArrayList<StatsFile> getFiles(Date from, Date till) {
    Calendar c = Calendar.getInstance();
    c.setTime(from);

    final ArrayList<StatsFile> res = new ArrayList<StatsFile>(10);

    while(c.getTime().before(till)) {
      StatsFile f = getFile(df.format(c.getTime()), false, false);
      if (f != null)
        res.add(f);
      c.set(Calendar.DATE, c.get(Calendar.DATE) + 1);
    }

    return res;
  }




  public void addDeliveryStat(String subscriberAddress, int advertiserId, Date date, int deliveredInc, int sendedInc) throws DataSourceException {
    StatsFile file = null;
    try {
      file = getFile(date, true);
      if (file != null) {
        DeliveryStat stat = new DeliveryStatImpl();
        stat.setSubscriberAddress(subscriberAddress);
        stat.setDelivered(deliveredInc);
        stat.setSended(sendedInc);
        stat.setAdvertiserId(advertiserId);
        file.addStat(stat);
      } else
        throw new DataSourceException("Null file for " + date);
    } catch (StatsFileException e) {
      throw new DataSourceException(e);
    } finally {
      if (file != null)
        try {
          file.close();
        } catch (StatsFileException e) {
          log.error(e,e);
        }
    }
  }

  public ResultSet<DeliveryStat> aggregateDeliveryStats(Date startDate, Date endDate, final DeliveryStatsQuery query) throws DataSourceException {
    ArrayList<StatsFile> files = getFiles(startDate, endDate);

    if (files.isEmpty())
      return null;

    final File aggregateFile = new File(storeDir, df.format(startDate) + '-' + df.format(endDate) + ".stats");

    final DeliveryStatsQuery query1 = new DeliveryStatsQuery() {
      public boolean isAllowed(DeliveryStat stat) {
        if (query.isAllowed(stat)) {
          stat.setAdvertiserId(0);
          return true;
        }
        return false;
      }
    };

    // Concat stat files
    OutputStream os = null;
    try {
      os = new BufferedOutputStream(new FileOutputStream(aggregateFile));

      for (StatsFile file : files)
        file.transferTo(query1, os);

    } catch (StatsFileException e) {
      throw new DataSourceException(e);
    } catch (IOException e) {
      throw new DataSourceException(e);
    } finally {
      try {
        if (os != null)
          os.close();
      } catch (IOException e) {
      }
    }

    // Aggregate final file
    StatsFileImpl impl = null;
    try {
      impl = new StatsFileImpl(aggregateFile);
      impl.compress();

      return new ListResultSet(impl) {
        public void close() throws DataSourceException {
          super.close();
          aggregateFile.delete();
        }
      };

    } catch (IOException e) {
      throw new DataSourceException(e);
    } catch (StatsFileException e) {
      try {
        impl.close();
      } catch (StatsFileException e1) {
        log.error(e,e);
      }
      throw new DataSourceException(e);
    }
  }

  public ResultSet<DeliveryStat> getDeliveryStats(Date date) throws DataSourceException {
    final StatsFile file = getFile(date, false);
    return (file == null) ? null : new ListResultSet(file);
  }

  public void shutdown() {
    cleaner.shutdownNow();
    compressor.shutdown();
    clearCache(true, false);
  }








  private static final class HashedStatsFile implements StatsFile {

    private final StatsFileImpl impl;
    private final boolean compressible;
    private final ReentrantLock lock = new ReentrantLock();
    private long time;
    private boolean opened;

    public HashedStatsFile(StatsFileImpl impl, boolean compressible) {
      this.impl = impl;
      this.compressible = compressible;
      this.opened = false;
      this.time = System.currentTimeMillis();
      open();
    }

    public String getName() {
      return impl.getName();
    }

    public void addStat(DeliveryStat stat) throws StatsFileException {
      try {
        lock.lock();
        time = System.currentTimeMillis();
        this.impl.addStat(stat);
      } finally {
        lock.unlock();
      }
    }

    public ArrayList<DeliveryStatImpl> getRecords(int start, int end) throws StatsFileException {
      try {
        lock.lock();
        time = System.currentTimeMillis();
        return this.impl.getRecords(start, end);
      } finally {
        lock.unlock();
      }
    }

    public void transferTo(DeliveryStatsQuery query, OutputStream target) throws StatsFileException {
      try {
        lock.lock();
        impl.transferTo(query, target);
        time = System.currentTimeMillis();
      } finally {
        lock.unlock();
      }
    }

    public void compress() throws StatsFileException {
      try {
        lock.lock();
        time = System.currentTimeMillis();
        this.impl.compress();
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

    public void open() {
      try {
        lock.lock();
        opened = true;
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

    public void close() {
      try {
        lock.lock();
        opened = false;
        time = System.currentTimeMillis();
      } finally {
        lock.unlock();
      }
    }
  }







  private static class ListResultSet implements ResultSet<DeliveryStat> {

    private final StatsFile file;
    private ArrayList<DeliveryStatImpl> buffer;
    private int start;
    private int pos;

    public ListResultSet(StatsFile file) {
      this.file = file;
      this.buffer = null;
      this.start = 0;
      this.pos = 0;
    }

    public DeliveryStat get() throws DataSourceException {
      return buffer.get(pos);
    }

    public boolean next() throws DataSourceException {
      if (buffer == null || pos+1 >= buffer.size()) {
        try {
          buffer = file.getRecords(start, 5000);
          if (buffer.isEmpty())
            return false;
          start += buffer.size();
          pos = 0;
        } catch (StatsFileException e) {
          throw new DataSourceException(e);
        }
      } else {
        pos++;
      }
      return true;
    }

    public void close() throws DataSourceException {
      try {
        file.close();
      } catch (StatsFileException e) {
        throw new DataSourceException(e);
      }
    }
  }





  private class FileCompressTask implements Runnable {

    private final HashedStatsFile file;
    private final String key;

    public FileCompressTask(HashedStatsFile file, String key) {
      this.file = file;
      this.key = key;
    }

    public void run() {
      try {
        file.compress();
      } catch (StatsFileException e) {
        log.error(e,e);
      }

      try {
        cacheLock.lock();
        cache.remove(key);
      } finally {
        cacheLock.unlock();
      }
    }
  }




  public static void main(String args[]) {
    final FileDeliveryStatDataSource ds = new FileDeliveryStatDataSource("store");

    try {
      Calendar c = Calendar.getInstance();
      c.setTimeInMillis(System.currentTimeMillis());
      for (int k=0; k<10; k++) {
        Date date = c.getTime();
        for (int i = 0; i < k; i++) {
          for (int j = 100000; j < 300002; j++)
            ds.addDeliveryStat("+79139" + j, 1, date, 1, 1);
        }
        c.set(Calendar.DATE, c.get(Calendar.DATE) + 1);
      }

      final Object o = new Object();

      synchronized(o) {
        try {
          o.wait(5000);
        } catch (InterruptedException e) {          
        }
      }

    } catch (DataSourceException e) {
      e.printStackTrace();
    } finally {
      ds.shutdown();
    }
  }

}

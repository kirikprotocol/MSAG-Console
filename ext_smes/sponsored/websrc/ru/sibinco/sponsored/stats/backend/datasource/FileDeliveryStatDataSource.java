package ru.sibinco.sponsored.stats.backend.datasource;

import org.apache.log4j.Category;
import ru.sibinco.sponsored.stats.backend.Progress;
import ru.sibinco.sponsored.stats.backend.StatisticsException;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.channels.FileChannel;
import java.nio.channels.WritableByteChannel;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class FileDeliveryStatDataSource {

  private static final Category log = Category.getInstance(FileDeliveryStatDataSource.class);

  private final Map cache = new HashMap(10);

  private final Object hashLock = new Object();

  private final File storeDir;


  public FileDeliveryStatDataSource(File storeDir) {
    this.storeDir = storeDir;
  }

  StatsFile getFile(Date date, boolean create) {
    SimpleDateFormat df = new SimpleDateFormat("yyyyMMdd");
    return getFile(df.format(date), create);
  }

  private StatsFile getFile(String id, boolean create) {
    try {
      synchronized (hashLock) {
        HashedStatsFile page = (HashedStatsFile)cache.get(id);
        if (page == null) {
          File f = new File(storeDir, id + ".stats");
          if (!create && !f.exists())
            return null;
          page = new HashedStatsFile(new StatsFileImpl(f));
          cache.put(id, page);
        }
        page.open();
        return page;
      }
    } catch (IOException e) {
      return null;
    }
  }

  private  ArrayList getFiles(Date from, Date till) {
    SimpleDateFormat df = new SimpleDateFormat("yyyyMMdd");
    Calendar c = Calendar.getInstance();
    c.setTime(from);

    final ArrayList res = new ArrayList(10);

    while(c.getTime().before(till)) {
      StatsFile f = getFile(df.format(c.getTime()), false);
      if (f != null)
        res.add(f);
      c.set(Calendar.DATE, c.get(Calendar.DATE) + 1);
    }
    return res;
  }

  public ResultSet aggregateDeliveryStats(Date startDate, Date endDate, final DeliveryStatsQuery query, final Progress progress) throws StatisticsException {
    if(startDate == null) {
      startDate = new Date(0);
    }
    if(endDate == null) {
      endDate = new Date();
    }
    ArrayList files = getFiles(startDate, endDate);

    progress.setProgress(10);
    if(Thread.currentThread().isInterrupted()) {
      throw new StatisticsException(StatisticsException.Code.INTERRUPTED);
    }

    if (files.isEmpty()) {
      progress.setProgress(100);
      return null;
    }

    SimpleDateFormat df = new SimpleDateFormat("yyyyMMdd");

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
    FileOutputStream os = null;
    FileChannel fc = null;
    try {
      os = new FileOutputStream(aggregateFile);
      fc = os.getChannel();

      Iterator i = files.iterator();
      while(i.hasNext()) {
        StatsFile file = (StatsFile)i.next();
        file.transferTo(fc);
      }
      progress.setProgress(50);

    } catch (IOException e) {
      throw new StatisticsException(e);
    } finally {
      try {
        if (os != null)
          os.close();
      } catch (IOException ignored) {
      }
      try {
        if (fc != null)
          fc.close();
      } catch (IOException ignored) {
      }
    }
    if(Thread.currentThread().isInterrupted()) {
      throw new StatisticsException(StatisticsException.Code.INTERRUPTED);
    }

    // Aggregate final file
    StatsFileImpl impl = null;
    try {
      impl = new StatsFileImpl(aggregateFile);
      impl.compress(query1);

      progress.setProgress(70);

      if(Thread.currentThread().isInterrupted()) {
        throw new StatisticsException(StatisticsException.Code.INTERRUPTED);
      }


      return new ListResultSet(impl) {

        public boolean next() throws StatisticsException {
          if(Thread.currentThread().isInterrupted()) {
            throw new StatisticsException(StatisticsException.Code.INTERRUPTED);
          }
          return super.next();
        }

        public void close() throws StatisticsException {
          try{
            super.close();
            if(!aggregateFile.delete()) {
              log.warn("Can't remove: "+aggregateFile.getAbsolutePath());
            }
          }finally {
            progress.setProgress(100);
          }
        }
      };

    } catch (IOException e) {
      throw new StatisticsException(e);
    } catch (StatisticsException e) {
      try {
        impl.close();
      } catch (Exception e1) {
        log.error(e,e);
      }
      throw new StatisticsException(e);
    }
  }

  private static final class HashedStatsFile implements StatsFile {

    private final StatsFileImpl impl;
    private final Object lock = new Object();
    private long time;
    private boolean opened;

    public HashedStatsFile(StatsFileImpl impl) {
      this.impl = impl;
      this.opened = false;
      this.time = System.currentTimeMillis();
      open();
    }

    public String getName() {
      return impl.getName();
    }

    public void addStat(DeliveryStat stat) throws StatisticsException {
      synchronized (lock) {
        time = System.currentTimeMillis();
        this.impl.addStat(stat);
      }
    }

    public ArrayList getRecords(int start, int end) throws StatisticsException {
      synchronized (lock) {
        time = System.currentTimeMillis();
        return this.impl.getRecords(start, end);
      }
    }

    public void transferTo(DeliveryStatsQuery query, OutputStream target) throws StatisticsException {
      synchronized (lock) {
        impl.transferTo(query, target);
        time = System.currentTimeMillis();
      }
    }

    public void transferTo(WritableByteChannel target) throws StatisticsException {
      synchronized (lock) {
        impl.transferTo(target);
        time = System.currentTimeMillis();
      }
    }

    public void compress(DeliveryStatsQuery query) throws StatisticsException {
      synchronized (lock) {
        time = System.currentTimeMillis();
        this.impl.compress(query);
      }
    }

    public void open() {
      synchronized (lock) {
        opened = true;
        time = System.currentTimeMillis();
      }
    }
    public void close() {
      synchronized (lock) {
        opened = false;
        time = System.currentTimeMillis();
      }
    }
  }

  private static class ListResultSet implements ResultSet {

    private final StatsFile file;
    private ArrayList buffer;
    private int start;
    private int pos;

    public ListResultSet(StatsFile file) {
      this.file = file;
      this.buffer = null;
      this.start = 0;
      this.pos = 0;
    }

    public Object get() {
      return buffer.get(pos);
    }

    public boolean next() throws StatisticsException {
      if (buffer == null || pos+1 >= buffer.size()) {
        try {
          buffer = file.getRecords(start, 5000);
          if (buffer.isEmpty())
            return false;
          start += buffer.size();
          pos = 0;
        } catch (StatisticsException e) {
          throw new StatisticsException(e);
        }
      } else {
        pos++;
      }
      return true;
    }

    public void close() throws StatisticsException {
      try {
        file.close();
      } catch (StatisticsException e) {
        throw new StatisticsException(e);
      }
    }
  }

}

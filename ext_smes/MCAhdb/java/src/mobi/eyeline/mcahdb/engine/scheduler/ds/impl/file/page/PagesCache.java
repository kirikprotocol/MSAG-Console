package mobi.eyeline.mcahdb.engine.scheduler.ds.impl.file.page;

import mobi.eyeline.mcahdb.engine.DataSourceException;
import mobi.eyeline.mcahdb.engine.scheduler.ds.impl.file.page.SchedulePage;
import mobi.eyeline.mcahdb.engine.scheduler.ds.Task;
import mobi.eyeline.mcahdb.engine.scheduler.ds.impl.file.page.SchedulePageImpl;

import java.util.*;
import java.util.regex.Pattern;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.*;
import java.io.File;
import java.io.IOException;
import java.io.FilenameFilter;
import java.text.SimpleDateFormat;
import java.text.ParseException;

import org.apache.log4j.Category;

/**
 * User: artem
 * Date: 27.08.2008
 */

public class PagesCache {

  private static final Category log = Category.getInstance(PagesCache.class);
  private static final SimpleDateFormat df = new SimpleDateFormat("yyyyMM/dd/HH");

  private static final int CACHE_CLEAN_INTERVAL = 600000;
  private static final int CACHE_MODIFIED_CLOSE_INTERVAL = 600000;

  private final File storeDir;
  private final ScheduledExecutorService cleaner;
  private final ConcurrentHashMap<String, CachedPageImpl> cache = new ConcurrentHashMap<String, CachedPageImpl>(10);

  public PagesCache(File storeDir) {
    this.storeDir = storeDir;

    this.cleaner = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
      public Thread newThread(Runnable r) {
        return new Thread(r, "Schedule-Page-Cache-Cleaner");
      }
    });
    this.cleaner.scheduleWithFixedDelay(new Runnable() {
      public void run() {
        clearCache(false);
      }
    }, CACHE_MODIFIED_CLOSE_INTERVAL, CACHE_MODIFIED_CLOSE_INTERVAL, TimeUnit.MILLISECONDS);
  }

  private void clearCache(boolean all) {
    try {

      if (log.isDebugEnabled())
        log.debug("Clear cache called: " + all);

      Map.Entry<String, CachedPageImpl> e;
      for (Iterator<Map.Entry<String, CachedPageImpl>> entries = cache.entrySet().iterator(); entries.hasNext();) {
        e = entries.next();

        if (e.getValue().closeInt(all))
          entries.remove();
      }

    } catch (Throwable e) {
      log.error("Clear cache err.", e);
    }
  }

  private static String getPageId(Date date) {
    return df.format(date) + ".csv";
  }

  private static Collection<String> getPageIds(Date from, Date till) {
    Collection<String> result = new ArrayList<String>((int)(till.getTime() - from.getTime()) / 3600000 + 2);
    long cur = from.getTime();
    while (cur <= till.getTime()) {
      result.add(getPageId(new Date(cur)));
      cur += 3600000;
    }
    return result;
  }

  public SchedulePage getPageByDate(Date date, boolean create) throws DataSourceException {
    return getPage(getPageId(date), create);
  }

  public SchedulePage getPageById(String id) throws DataSourceException {
    return getPage(id, false);
  }

  public void getPagesByDates(Date from, Date till, Collection<SchedulePage> result) throws DataSourceException {
    for (String id : getPageIds(from, till)) {
      SchedulePage p = getPage(id, false);
      if (p != null)
        result.add(p);    
    }
  }

  public void getPagesFromDate(Date from, Collection<SchedulePage> result) throws DataSourceException {
    final Pattern p = Pattern.compile(".*csv");
    File[] files = storeDir.listFiles(new FilenameFilter() {
      public boolean accept(File dir, String name) {
        return p.matcher(name).matches();
      }
    });

    try {
      String fromStr = df.format(from);
      Date fromDate = df.parse(fromStr);
      for (File f : files) {
        String name = f.getName();
        String dateStr = name.substring(0, name.lastIndexOf('.'));

        Date date = df.parse(dateStr);
        if (date.getTime() >= fromDate.getTime()) {
          SchedulePage page = getPage(dateStr, false);
          if (page != null)
            result.add(page);
        }
      }
    } catch (ParseException e) {
      throw new DataSourceException(e);
    }
  }

  private SchedulePage getPage(String id, boolean create) throws DataSourceException {
    try {
      CachedPageImpl page = cache.get(id);
      if (page == null) {
        File f = new File(storeDir, id);
        if (!f.exists()) {
          if (!create)
            return null;
          File parent = f.getParentFile();
          if (!parent.exists() && !parent.mkdirs())
            throw new DataSourceException("Can't create dir: " + parent.getAbsolutePath());
        }
        page = new CachedPageImpl(new SchedulePageImpl(f, id));
        CachedPageImpl prev = cache.putIfAbsent(id, page);
        if (prev != null)
          page = prev;
      }      
      return page;
    } catch (IOException e) {
      throw new DataSourceException(e);
    }
  }

  public void shutdown() {
    cleaner.shutdownNow();
    clearCache(true);
  }


  private static class CachedPageImpl implements SchedulePage {

    private final Lock lock = new ReentrantLock();
    private final SchedulePageImpl impl;

    private long closeTime;
    private boolean opened = false;

    private CachedPageImpl(SchedulePageImpl impl) {
      this.impl = impl;
    }

    public void open() throws DataSourceException {
      lock.lock();
      if (!opened) {
        try {
        impl.open();
        } catch (Throwable e) {
          lock.unlock();
          throw new DataSourceException(e);
        }
        opened = true;
      }
    }

    public String getId() {
      return impl.getId();
    }

    public TaskPointer add(Task task) throws DataSourceException {
      if (opened)
        return impl.add(task);
      throw new DataSourceException("File is not opened");
    }

    public Task remove(long taskId) throws DataSourceException {
      if (opened)
        return impl.remove(taskId);
      else
        throw new DataSourceException("File is not opened");
    }

    public Task get(long taskId) throws DataSourceException {
      if (opened)
        return impl.get(taskId);
      throw new DataSourceException("File is not opened");
    }

    public void list(Date from, Date till, Collection<Task> result) throws DataSourceException {
      if (opened)
        impl.list(from, till, result);
      else
        throw new DataSourceException("File is not opened");
    }

    public boolean closeInt(boolean all) {
      try {
        lock.lock();
        if (opened || ((System.currentTimeMillis() - closeTime) < CACHE_CLEAN_INTERVAL & !all))
          return false;

        impl.close();
        return true;
      } finally {
        lock.unlock();
      }
    }

    public void close() {
      opened = false;
      closeTime = System.currentTimeMillis();
      lock.unlock();
    }
  }
}

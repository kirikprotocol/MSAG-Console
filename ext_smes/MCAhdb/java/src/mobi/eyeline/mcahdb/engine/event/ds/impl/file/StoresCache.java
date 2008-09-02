package mobi.eyeline.mcahdb.engine.event.ds.impl.file;

import mobi.eyeline.mcahdb.engine.event.ds.impl.file.StoreImpl;
import mobi.eyeline.mcahdb.engine.event.ds.Event;
import mobi.eyeline.mcahdb.engine.DataSourceException;

import java.util.*;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.text.SimpleDateFormat;
import java.io.File;
import java.io.IOException;

import org.apache.log4j.Category;

/**
 * User: artem
 * Date: 31.07.2008
 */

public class StoresCache {

  private static final Category log = Category.getInstance(StoresCache.class);

  private static final SimpleDateFormat fileNameFormat = new SimpleDateFormat("yyMM/dd/HH");

  private static final int CACHE_CLEAN_INTERVAL = 3600000;
  private static final int CACHE_MODIFIED_CLOSE_INTERVAL = 600000;

  private final File storeDir;
  private final ScheduledExecutorService cleaner;
  private final HashMap<String, CachedStore> cache = new HashMap<String, CachedStore>(10);
  private final Lock cacheLock = new ReentrantLock();

  public StoresCache(File storeDir) throws DataSourceException {
    this.storeDir = storeDir;
    if (!storeDir.exists() && !storeDir.mkdirs())
      throw new DataSourceException("Cant create store dir: " + storeDir.getAbsolutePath());

    this.cleaner = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
      public Thread newThread(Runnable r) {
        return new Thread(r, "Stores-Cache-Cleaner");
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
      cacheLock.lock();

      if (log.isDebugEnabled())
        log.debug("Clear cache called: " + all);

      Map.Entry<String, CachedStore> e;
      for (Iterator<Map.Entry<String, CachedStore>> entries = cache.entrySet().iterator(); entries.hasNext();) {
        e = entries.next();

        try {
          if (e.getValue().closeInt(all))
            entries.remove();
        } catch (IOException e1) {
          log.error("Can't close store", e1);
        }
      }

    } finally {
      cacheLock.unlock();
    }
  }

  private Store getFile(String name, boolean readOnly) throws DataSourceException {
    try {
      cacheLock.lock();

      CachedStore page = cache.get(name);

      if (page != null && page.isReadOnly() && !readOnly) {
        page.closeInt(true);
        cache.remove(name);        
        page = null;
      }

      if (page == null) {
        File f = new File(storeDir, name);
        if (!f.exists()) {
          if (readOnly)
            return null;
          File parent = f.getParentFile();
          if (!parent.exists() && !parent.mkdirs())
            throw new DataSourceException("Can't create dir: " + parent.getAbsolutePath());
        }
        page = new CachedStore(new StoreImpl(f, readOnly));
        cache.put(name, page);
      }

      return page;
    } catch (IOException e) {
      throw new DataSourceException(e);
    } finally {
      cacheLock.unlock();
    }
  }

  private static String getFileName(Date date) {
    return fileNameFormat.format(date) + ".dat";
  }

  public Collection<Store> getFilesForRead(Date from, Date till) throws DataSourceException {
    Collection<Store> result = new LinkedList<Store>();

    long time = from.getTime();
    long tillTime = till.getTime();
    while(time <= tillTime) {
      Date date = new Date(time);
      Store s = getFile(getFileName(date), true);
      if (s != null)
        result.add(s);      
      time += 3600000;
    }

    return result;
  }

  public Store getFileForWrite(Date date) throws DataSourceException {
    return getFile(getFileName(date), false);
  }

  public void shutdown() {
    log.warn("Shutdown called.");
    cleaner.shutdownNow();
    clearCache(true);
  }



  private static class CachedStore implements Store {

    private final StoreImpl impl;
    private final Lock lock = new ReentrantLock();

    private boolean opened = false;

    private long closeTime;
    private boolean modified = false;

    private CachedStore(StoreImpl impl) {
      this.impl = impl;
    }

    public void open() throws DataSourceException {
      lock.lock();  // lock store until close()
      if (!opened) {
        impl.open();
        opened = true;
      }
    }

    public void close() throws IOException {
      closeTime = System.currentTimeMillis();
      lock.unlock(); // unlock store
    }

    public boolean closeInt(boolean f) throws IOException {
      try {
        lock.lock();

        long now = System.currentTimeMillis();

        if (f || (modified && now - closeTime > CACHE_MODIFIED_CLOSE_INTERVAL) || (now - closeTime > CACHE_CLEAN_INTERVAL)) {
          impl.close();
          opened = false;
          return true;
        }

        return false;
      } finally {
        lock.unlock();
      }
    }

    public boolean isReadOnly() {
      return impl.isReadOnly();
    }

    public void commit() throws DataSourceException, IOException {
      impl.commit();
    }

    public void rollback() throws DataSourceException, IOException {
      impl.rollback();
    }

    public void getEvents(String address, Date from, Date till, Collection<Event> result) throws DataSourceException, IOException {
      impl.getEvents(address, from, till, result);
    }

    public void addEvent(Event event) throws DataSourceException, IOException {
      impl.addEvent(event);
      modified = true;
    }

    public int hashCode() {
      return impl.hashCode();
    }

    public boolean equals(Object o) {
      if (o instanceof CachedStore)
        return ((CachedStore)o).impl.equals(impl);
      return false;
    }
  }
}

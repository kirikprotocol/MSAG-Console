package mobi.eyeline.mcahdb.engine.scheduler.ds.impl.file;

import mobi.eyeline.mcahdb.engine.scheduler.ds.impl.file.page.SchedulePage;
import mobi.eyeline.mcahdb.engine.scheduler.ds.impl.file.page.PagesCache;
import mobi.eyeline.mcahdb.engine.scheduler.ds.impl.file.page.TaskPointer;
import mobi.eyeline.mcahdb.engine.scheduler.ds.TaskDataSource;
import mobi.eyeline.mcahdb.engine.scheduler.ds.Task;
import mobi.eyeline.mcahdb.engine.DataSourceException;

import java.util.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.io.File;

/**
 * User: artem
 * Date: 28.08.2008
 */

public class TaskDataSourceImpl implements TaskDataSource {

  private PagesCache cache;
  private TaskIndex index = new TaskIndex();

  public TaskDataSourceImpl(File storeDir, Date startDate) throws DataSourceException {
    if (!storeDir.exists() && !storeDir.mkdirs())
      throw new  DataSourceException("Can't create store dir for scheduler: " + storeDir.getAbsolutePath());
    
    this.cache = new PagesCache(storeDir);    
    restoreIndex(startDate);
  }

  private void restoreIndex(Date startDate) throws DataSourceException {
    Date tillDate = new Date(Long.MAX_VALUE);
    LinkedList<SchedulePage> pages = new LinkedList<SchedulePage>();
    cache.getPagesFromDate(startDate, pages);
    for (SchedulePage page : pages) {
      try {
        page.open();

        page.list(startDate, tillDate, index);

      } finally {
        page.close();
      }
    }
  }

  public void add(Task task) throws DataSourceException {    
    // Add new task
    SchedulePage page = null;
    try {
      page = cache.getPageByDate(task.getTime(), true);
      page.open();

      TaskPointer pp = page.add(task);
      index.put(task.getCaller(), task.getCalled(), pp);
      task.setId(pp);

    } finally {
      if (page != null)
        page.close();
    }
  }

  public void remove(Task task) throws DataSourceException {
    if (task.getId() != null)
      remove((TaskPointer)task.getId());
    else
      remove(task.getCaller(), task.getCalled());
  }

  private Task remove(TaskPointer fp) throws DataSourceException {
    SchedulePage page = null;
    try {
      page = cache.getPageById(fp.getPageId());
      if (page != null) {
        page.open();
        return page.remove(fp.getPointer());
      } else
        return null;      

    } finally {
      if (page != null)
        page.close();
    }
  }

  public Task remove(String caller, String called) throws DataSourceException {
    TaskPointer fp = index.remove(caller, called);
    if (fp == null)
      return null;

    return remove(fp);
  }

  public Task get(String caller, String called) throws DataSourceException {
    TaskPointer fp = index.get(caller, called);
    if (fp == null)
      return null;

    SchedulePage page = null;
    try {
      page = cache.getPageById(fp.getPageId());

      if (page != null) {
        page.open();
        return page.get(fp.getPointer());
      } else
        throw new DataSourceException("Page not found: " + fp.getPageId());

    } finally {
      if (page != null)
        page.close();
    }
  }

  public void get(String called, Collection<Task> result) throws DataSourceException {
    TreeSet<TaskPointer> pointers = new TreeSet<TaskPointer>();
    index.get(called, pointers);

    SchedulePage currentPage = null;
    try {
      for (TaskPointer tp : pointers) {
        SchedulePage p = cache.getPageById(tp.getPageId());
        if (p == null)
          continue;

        if (currentPage != p) {
          if (currentPage != null)
            currentPage.close();
          currentPage = p;
          currentPage.open();
        }

        Task t = currentPage.get(tp.getPointer());

        if (t != null)
          result.add(t);
      }
    } finally {
      if (currentPage != null)
        currentPage.close();
    }
  }

  public void list(Date fromDate, Date tillDate, Collection<Task> result) throws DataSourceException {
    LinkedList<SchedulePage> pages = new LinkedList<SchedulePage>();
    cache.getPagesByDates(fromDate, tillDate, pages);
    for (SchedulePage page : pages) {
      try {
        page.open();

        page.list(fromDate, tillDate, result);

      } finally {
        page.close();
      }
    }
  }

  /**
   *
   */
  private static class TaskKey {
    private final String caller;
    private final String called;

    private TaskKey(String caller, String called) {
      this.caller = caller;
      this.called = called;
    }

    public int hashCode() {
      return caller.hashCode() + called.hashCode();
    }

    public boolean equals(Object o) {
      TaskKey k = (TaskKey)o;
      return k.caller.equals(caller) && k.called.equals(called);
    }
  }

  /**
   *
   */
  private static final class TaskIndex implements Collection<Task> {
    private final Lock lock = new ReentrantLock();
    private final Map<TaskKey, TaskPointer> index = new HashMap<TaskKey, TaskPointer>(1000);

    public void put(String caller, String called, TaskPointer p) {
      try {
        lock.lock();
        index.put(new TaskKey(caller, called), p);
      } finally {
        lock.unlock();
      }
    }

    public TaskPointer get(String caller, String called) {
      try {
        lock.lock();
        return index.get(new TaskKey(caller, called));
      } finally {
        lock.unlock();
      }
    }

    public void get(String called, Collection<TaskPointer> result) {
      try {
        lock.lock();

        for (Map.Entry<TaskKey, TaskPointer> e : index.entrySet()) 
          if (e.getKey().called.equals(called))
            result.add(e.getValue());
      } finally {
        lock.unlock();
      }
    }

    public TaskPointer remove(String caller, String called) {
      try {
        lock.lock();
        return index.remove(new TaskKey(caller, called));
      } finally {
        lock.unlock();
      }
    }

    public int size() {
      return 0;
    }

    public boolean isEmpty() {
      return false;
    }

    public boolean contains(Object o) {
      return false;
    }

    public Iterator<Task> iterator() {
      return null;
    }

    public Object[] toArray() {
      return new Object[0];
    }

    public <T> T[] toArray(T[] a) {
      return null;
    }

    public boolean add(Task o) {
      try {
        lock.lock();
        index.put(new TaskKey(o.getCaller(), o.getCalled()), (TaskPointer)o.getId());
      } finally {
        lock.unlock();
      }
      return true;
    }

    public boolean remove(Object o) {
      return false;
    }

    public boolean containsAll(Collection<?> c) {
      return false;
    }

    public boolean addAll(Collection<? extends Task> c) {
      return false;
    }

    public boolean removeAll(Collection<?> c) {
      return false;
    }

    public boolean retainAll(Collection<?> c) {
      return false;
    }

    public void clear() {
    }
  }
}

package ru.novosoft.smsc.infosme.backend.config.tasks;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.config.Changes;
import ru.novosoft.smsc.util.config.Config;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.channels.FileChannel;
import java.util.*;

/**
 * User: artem
 * Date: 19.05.2008
 */

public class TaskManager {

  private static final Category logger = Category.getInstance(TaskManager.class);

  public static final String TASKS_PREFIX = "InfoSme.Tasks";

  private int id;
  private final Object idlock = new Object();
  private RandomAccessFile idFile;

  private final Map tasks = new HashMap(100);
  private boolean modified = false;

  public TaskManager(String configDir, Config config) throws AdminException {
    try {
      idFile = new RandomAccessFile(new File(configDir, "taskid.bin"), "rw");
      long len = idFile.length();
      if (len % 4 > 0) {
        logger.warn("File taskid.bin is broken and will be repaired.");
        truncateFile(idFile, 4 * len / 4 - 1);
        idFile.close();
        idFile = new RandomAccessFile(new File(configDir, "taskid.bin"), "rw");
        logger.warn("File taskid.bin was successfully repaired.");
      }

      // Load id
      if (len == 0)
        id = 0;
      else {
        idFile.seek(idFile.length() - 4);
        id = idFile.readInt();
      }

      resetTasks(null, config);
    } catch (Exception e) {
      e.printStackTrace();
      throw new AdminException("Can't init task manager");
    }
  }

  private static void truncateFile(RandomAccessFile f, long index) throws IOException {
    FileChannel fc = null;
    try {
      fc = f.getChannel();
      fc.truncate(index);
    } finally {
      if (fc != null)
        fc.close();
    }
  }

  private int getId() throws AdminException {
    synchronized(idlock) {
      try {
        id++;
        idFile.seek(idFile.length());
        idFile.writeInt(id);
        return id;
      } catch (IOException e) {
        throw new AdminException(e.getMessage());
      }
    }
  }

  private static List loadTasks(Config config) throws Config.WrongParamTypeException, Config.ParamNotFoundException {
    List result = new ArrayList(100);
    for (Iterator i = config.getSectionChildShortSectionNames(TASKS_PREFIX).iterator(); i.hasNext();)
      result.add(new Task(config, (String)i.next()));
    return result;
  }

  public synchronized Task createTask() throws AdminException {
    return new Task(String.valueOf(getId()));
  }

  public synchronized void addTask(Task t) {
    tasks.put(t.getId(), t);
  }

  public synchronized void addTask(Task t, Config cfg) throws AdminException {
    try {
      addTask(t);
      t.storeToConfig(cfg);
      t.setModified(false);
    } catch (Exception e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
  }

  public synchronized void removeTask(String id) {
    tasks.remove(id);
    modified = true;
  }

  public synchronized boolean removeTask(String id, Config cfg) throws AdminException {
    try {
      boolean removed = tasks.remove(id) != null;
      if (removed)
        cfg.removeSection(TASKS_PREFIX + '.' + id);
      return removed;
    } catch (Exception e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
  }

  public synchronized Task getTask(String id) {
    return (Task)tasks.get(id);
  }

  public synchronized List getTasks(String owner) {
    List result = new ArrayList(tasks.size());
    for (Iterator iter = tasks.values().iterator(); iter.hasNext();) {
      Task t = (Task)iter.next();
      if (owner == null || owner.equals(t.getOwner()))
        result.add(t);
    }
    return result;

  }

  public synchronized boolean containsTaskWithId(String id) {
    return tasks.containsKey(id);
  }

  public synchronized boolean containsTaskWithName(String name) {
    for (Iterator iter = tasks.values().iterator(); iter.hasNext();) {
      if (((Task)iter.next()).getName().equals(name))
        return true;
    }
    return false;
  }

  private Changes analyzeChanges(List oldTasks) throws Config.WrongParamTypeException, Config.ParamNotFoundException {
    Changes changes = new Changes();

    // Lookup new tasks
    for (Iterator iter = tasks.keySet().iterator(); iter.hasNext();) {
      String id = (String)iter.next();

      boolean contains = false;
      for (Iterator iter1 = oldTasks.iterator(); iter1.hasNext();) {
        Task t = (Task)iter1.next();
        if (t.getId().equals(id)) {
          contains = true;
          break;
        }
      }

      if (!contains)
        changes.added(id);
    }

    // Lookup deleted tasks
    for (Iterator iter = oldTasks.iterator(); iter.hasNext();) {
      Task t = (Task)iter.next();
      if (!containsTaskWithId(t.getId()))
        changes.deleted(t.getId());
    }

    // Lookup modified tasks
    for (Iterator iter = tasks.values().iterator(); iter.hasNext();) {
      Task t = (Task)iter.next();
      if (t.isModified() && !changes.isAdded(t.getId()) && !changes.isDeleted(t.getId()))
        changes.modified(t.getId());
    }

    return changes;
  }

  public synchronized Changes applyTasks(String owner, Config cfg) throws AdminException {
    try {
      List oldTasks = loadTasks(cfg);

      // Analyze changes
      Changes changes = analyzeChanges(oldTasks);

      // Remove all tasks by owner
      for (Iterator iter = oldTasks.iterator(); iter.hasNext();) {
        Task t = (Task)iter.next();
        if (owner == null || (t.getOwner() != null && t.getOwner().equals(owner)))
          cfg.removeSection(TASKS_PREFIX + '.' + t.getId());
      }

      // Add new tasks
      for (Iterator iter = tasks.values().iterator(); iter.hasNext();) {
        Task t = (Task)iter.next();
        if (owner == null || t.getOwner().equals(owner))
          t.storeToConfig(cfg);
      }

      return changes;

    } catch (Exception e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
  }

  public void setModified(boolean modified, String owner) {
    this.modified = modified;
    for (Iterator iter = tasks.values().iterator(); iter.hasNext();) {
        Task t = (Task)iter.next();
        if (owner == null || t.getOwner().equals(owner))
          t.setModified(false);
      }
  }

  public synchronized void resetTasks(String owner, Config cfg) throws AdminException {
    try {
      // Remove tasks by owner
      for (Iterator iter = tasks.entrySet().iterator(); iter.hasNext();) {
        Map.Entry e = (Map.Entry)iter.next();
        Task t = (Task)e.getValue();
        if (owner == null || t.getOwner().equals(owner))
          iter.remove();
      }
      // Load old tasks
      List oldTasks = loadTasks(cfg);
      for (Iterator iter = oldTasks.iterator(); iter.hasNext();) {
        Task t = (Task)iter.next();
        if (owner == null || owner.equals(t.getOwner()))
          addTask(t);
      }
      modified = false;
    } catch (Exception e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
  }

  public synchronized boolean isTasksChanged(String owner) {
    if (modified)
      return true;
    for (Iterator iter = tasks.values().iterator(); iter.hasNext();) {
      Task t = (Task)iter.next();
      if ((owner == null || owner.equals(t.getOwner())) && t.isModified())
        return true;
    }
    return false;
  }
}

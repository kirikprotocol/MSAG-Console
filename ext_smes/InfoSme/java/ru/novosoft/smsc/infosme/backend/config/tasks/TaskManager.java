package ru.novosoft.smsc.infosme.backend.config.tasks;

import org.apache.log4j.Category;
import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.config.Changes;
import ru.novosoft.smsc.util.config.Config;

import javax.xml.parsers.ParserConfigurationException;
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
  private String storeLocation;
  private boolean ussdPushFeature; // not set from frontend (yet)
  private Integer entriesPerDirectory;

  public TaskManager(String configDir, Config config) throws AdminException {

    try {
      this.ussdPushFeature = config.getBool("InfoSme.ussdPushFeature");
    } catch (Exception e) {
      this.ussdPushFeature = false;
    }

    if (config.containsParameter("InfoSme.entriesPerDirectory"))
      try {
        entriesPerDirectory = new Integer(config.getInt("InfoSme.entriesPerDirectory"));
      } catch (Exception e) {
        throw new AdminException(e.getMessage(), e);
      }

    try {
      File f = new File(configDir, "taskid.bin");
      if (!f.exists()) {
        File parentDir = new File(configDir).getParentFile();
        if (parentDir != null) {
          File f1 = new File(parentDir, "taskid.bin");
          if (f1.exists())
            f = f1;
          }
      }

      idFile = new RandomAccessFile(f, "rw");

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

      this.storeLocation = config.getString("InfoSme.storeLocation");
      resetTasks(null, config);
      //this.storeLocation = storeLocation;

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
    synchronized (idlock) {
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

  private List loadTasks(Config config) throws IOException, ParserConfigurationException, SAXException,
      Config.WrongParamTypeException, Config.ParamNotFoundException {
    logger.info("Store location: '" + storeLocation + "'");
    String[] storeDirs = new File(storeLocation).list();
    Set tasksNames = config.getSectionChildShortSectionNames(TASKS_PREFIX);

    for (Iterator i = tasksNames.iterator(); i.hasNext();) {
      logger.info("Task from common " + i.next());
    }

    if (entriesPerDirectory == null) {
      for (int i = 0; i < storeDirs.length; ++i) {
        String configLocation = storeLocation + File.separatorChar + storeDirs[i];
        if (new File(configLocation).isDirectory() && Task.existsConfigFile(configLocation)) {
          tasksNames.add(storeDirs[i]);
          logger.info("Add from store dir " + storeDirs[i]);
        }
      }
    } else {
      for (int i = 0; i < storeDirs.length; ++i) {
        File volumeFile = new File(storeLocation + File.separatorChar + storeDirs[i]);
        File[] taskDirs = volumeFile.listFiles();
        if (taskDirs != null) {
          for (int j = 0; j < taskDirs.length; j++) {
            String configLocation = storeLocation + File.separator + volumeFile.getName() + File.separator + taskDirs[j].getName();
            if (new File(configLocation).isDirectory() && Task.existsConfigFile(configLocation)) {
              tasksNames.add(taskDirs[j].getName());
              logger.info("Add from store dir " + storeDirs[i]);
            }
          }
        }
      }
    }

    for (Iterator i = tasksNames.iterator(); i.hasNext();) {
      logger.info("All names " + i.next());
    }

    List result = new ArrayList(100);
    for (Iterator i = tasksNames.iterator(); i.hasNext();) {
      result.add(new Task(config, (String) i.next(), storeLocation, entriesPerDirectory));
    }
    return result;
  }

  public synchronized Task createTask() throws AdminException {
    Task t = new Task(storeLocation, entriesPerDirectory);
    if (ussdPushFeature) {
      t.setUseUssdPush(0);
    }
    return t;
  }

  public boolean hasUssdPushFeature() {
    return ussdPushFeature;
  }

  public synchronized void addTask(Task t) throws AdminException {
    if (t.getId() == null) {
      t.setId(String.valueOf(getId()));
    }
    tasks.put(t.getId(), t);
  }

  public synchronized void addTask(Task t, Config cfg) throws AdminException {
    try {
      addTask(t);
      t.storeToConfig();
      //t.storeToConfig(cfg);
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
      //boolean removed = tasks.remove(id) != null;
      Task task = (Task) tasks.remove(id);
      if (task != null) {
        task.remove(cfg);
        //cfg.removeSection(TASKS_PREFIX + '.' + id);
        return true;
      } else {
        return false;
      }
    } catch (Exception e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
  }

  public synchronized Task getTask(String id) {
    return (Task) tasks.get(id);
  }

  public synchronized Task getTaskByName(String name) throws AdminException {
    for (Iterator iter = tasks.values().iterator(); iter.hasNext();) {
      Task task = (Task) iter.next();
      if (task.getName().equals(name)) {
        return task;
      }
    }
    throw new AdminException("task " + name + " not found");
  }

  public synchronized List getTasks(String owner) {
    List result = new ArrayList(tasks.size());
    for (Iterator iter = tasks.values().iterator(); iter.hasNext();) {
      Task t = (Task) iter.next();
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
      if (((Task) iter.next()).getName().equals(name))
        return true;
    }
    return false;
  }

  public synchronized boolean containsTaskWithName(String name, String owner) {
    for (Iterator iter = tasks.values().iterator(); iter.hasNext();) {
      Task t = (Task) iter.next();
      if (t.getName().equals(name) && t.getOwner().equals(owner))
        return true;
    }
    return false;
  }

  private Changes analyzeChanges(List oldTasks) throws Config.WrongParamTypeException, Config.ParamNotFoundException {
    Changes changes = new Changes();
    Changes changedTasks = new Changes();

    // Lookup new tasks
    for (Iterator iter = tasks.keySet().iterator(); iter.hasNext();) {
      String id = (String) iter.next();

      boolean contains = false;
      Task t = null;
      for (Iterator iter1 = oldTasks.iterator(); iter1.hasNext();) {
        //Task t = (Task)iter1.next();
        t = (Task) iter1.next();
        if (t.getId().equals(id)) {
          contains = true;
          break;
        }
      }

      if (!contains) {
        changes.added(id);
        if (t != null) changedTasks.added(t);
      }

    }

    // Lookup deleted tasks
    for (Iterator iter = oldTasks.iterator(); iter.hasNext();) {
      Task t = (Task) iter.next();
      if (!containsTaskWithId(t.getId())) {
        changes.deleted(t.getId());
        changedTasks.deleted(t);
      }
    }

    // Lookup modified tasks
    for (Iterator iter = tasks.values().iterator(); iter.hasNext();) {
      Task t = (Task) iter.next();
      if (t.isModified() && !changes.isAdded(t.getId()) && !changes.isDeleted(t.getId())) {
        changes.modified(t.getId());
        changedTasks.modified(t);
      }
    }

    //return changes;
    return changedTasks;
  }

  public synchronized Changes applyTasks(String owner, Config cfg) throws AdminException {
    try {
      List oldTasks = loadTasks(cfg);

      // Analyze changes
      Changes changedTasks = analyzeChanges(oldTasks);
      Changes changes = new Changes();

      //deleted tasks
      for (Iterator iter = changedTasks.getDeleted().iterator(); iter.hasNext();) {
        Task t = (Task) iter.next();
        if (owner == null || (t.getOwner() != null && t.getOwner().equals(owner))) {
          t.remove(cfg);
        }
        changes.deleted(t.getId());
      }

      //changed tasks
      for (Iterator iter = changedTasks.getModified().iterator(); iter.hasNext();) {
        Task t = (Task) iter.next();
        if (owner == null || (t.getOwner() != null && t.getOwner().equals(owner))) {
          t.change(cfg);
        }
        changes.modified(t.getId());
      }

      //added tasks
      for (Iterator iter = changedTasks.getAdded().iterator(); iter.hasNext();) {
        Task t = (Task) iter.next();
        if (owner == null || t.getOwner().equals(owner))
          t.storeToConfig();
        changes.modified(t.getId());
      }

      /*
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
      */
      return changes;
    } catch (Exception e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
  }

  public void setModified(boolean modified, String owner) {
    this.modified = modified;
    for (Iterator iter = tasks.values().iterator(); iter.hasNext();) {
      Task t = (Task) iter.next();
      if (owner == null || t.getOwner().equals(owner))
        t.setModified(false);
    }
  }

  public synchronized void resetTasks(String owner, Config cfg) throws AdminException {
    try {
      // Remove tasks by owner
      for (Iterator iter = tasks.entrySet().iterator(); iter.hasNext();) {
        Map.Entry e = (Map.Entry) iter.next();
        Task t = (Task) e.getValue();
        if (owner == null || t.getOwner().equals(owner))
          iter.remove();
      }
      // Load old tasks
      List oldTasks = loadTasks(cfg);
      for (Iterator iter = oldTasks.iterator(); iter.hasNext();) {
        Task t = (Task) iter.next();
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
      Task t = (Task) iter.next();
      if ((owner == null || owner.equals(t.getOwner())) && t.isModified())
        return true;
    }
    return false;
  }
}

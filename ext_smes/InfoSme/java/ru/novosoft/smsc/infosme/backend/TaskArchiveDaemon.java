package ru.novosoft.smsc.infosme.backend;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.preferences.UserPreferences;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.infosme.backend.config.tasks.Task;
import ru.novosoft.smsc.jsp.SMSCAppContext;

import java.util.Iterator;

/**
 * alkhal: alkhal
 */
public class TaskArchiveDaemon implements Runnable{

  private Category logger = Category.getInstance(TaskArchiveDaemon.class);

  private final InfoSmeContext infoSmeContext;

  private final SMSCAppContext smscAppContext;

  private boolean shutdown = true;

  private static final long TIMEOUT =  60000;

  public TaskArchiveDaemon(InfoSmeContext infoSmeContext, SMSCAppContext smscAppContext) {
    this.infoSmeContext = infoSmeContext;
    this.smscAppContext = smscAppContext;
  }


  public synchronized void shutdown() {
    shutdown = true;
  }

  public boolean isOnline() {
    return !shutdown;
  }

  public synchronized void start() {
    if(shutdown) {
      new Thread(this, "TaskArchiveDaemon").start();
      if (logger.isDebugEnabled()) {
        logger.debug("TaskArchiveDaemon started");
      }
    }
  }

  public void run() {

    try {
      shutdown = false;
      while (!shutdown) {
        try {
          Iterator taskIterator = infoSmeContext.getInfoSmeConfig().getTasks(null).iterator();
          while(taskIterator.hasNext()) {
            Task t = (Task)taskIterator.next();
            User owner = smscAppContext.getUserManager().getUser(t.getOwner());
            UserPreferences prefs = owner.getPrefs();
            if(prefs.isInfosmeArchive() &&
                (t.getStartDate().getTime() + (60*60*1000*prefs.getInfosmeArchiveTimeout()) <  System.currentTimeMillis())) {
              logger.debug("Task will be replaced in archive: "+t.getId()+" ("+t.getName()+')');
              infoSmeContext.getInfoSmeConfig().archivateAndApplyTask(t.getOwner(), t.getId());
              infoSmeContext.getInfoSme().removeTask(t.getId());
            }
          }

        } catch (Throwable e) {
          logger.error(e, e);
        }
        try {
          Thread.sleep(TIMEOUT);
        } catch (InterruptedException ignored) {
        }
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: task manager stopped");
      }

    } catch (Throwable e) {
      logger.error(e, e);
    }
  }
}

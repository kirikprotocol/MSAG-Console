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
  private Thread currentThread;

  private static final long TIMEOUT =  60000;

  public TaskArchiveDaemon(InfoSmeContext infoSmeContext, SMSCAppContext smscAppContext) {
    this.infoSmeContext = infoSmeContext;
    this.smscAppContext = smscAppContext;
  }


  public synchronized void shutdown() {
    shutdown = true;
    currentThread.interrupt();
  }

  public boolean isOnline() {
    return !shutdown;
  }

  public synchronized void start() {
    if(shutdown) {
      currentThread = new Thread(this, "TaskArchiveDaemon");
      currentThread.start();
    }
  }

  public void run() {

    try {
      shutdown = false;
      if (logger.isDebugEnabled()) {
        logger.debug("TaskArchiveDaemon started.");
      }
      while (!shutdown) {
        try {
          Iterator taskIterator = infoSmeContext.getInfoSmeConfig().getTasks(null).iterator();
          while(taskIterator.hasNext() && !shutdown) {
            Task t = (Task)taskIterator.next();
            User owner = smscAppContext.getUserManager().getUser(t.getOwner());
            UserPreferences prefs = owner.getPrefs();
            if(prefs.isInfosmeArchive() &&
                (t.getStartDate().getTime() + (60*60*1000*prefs.getInfosmeArchiveTimeout()) <  System.currentTimeMillis())) {

              if (logger.isDebugEnabled())
                logger.debug("Moving task to archive: "+t.getId()+" ("+t.getName()+")...");

              infoSmeContext.getInfoSmeConfig().archivateAndApplyTask(t.getOwner(), t.getId());
              infoSmeContext.getInfoSme().removeTask(t.getId());

              if (logger.isDebugEnabled())
                logger.debug("Task was successfully moved to archive: " + t.getId() + '(' + t.getName() + ").");
            }

            if (!shutdown) {
              try {
                Thread.sleep(1000);
              }catch (InterruptedException ignored) {}
            }
          }

        } catch (Throwable e) {
          logger.error(e, e);
        }

        if (!shutdown) {
          try {
            Thread.sleep(TIMEOUT);
          } catch (InterruptedException ignored) {
          }
        }
      }

      if (logger.isDebugEnabled()) {
        logger.debug("TaskArchiveDaemon stopped.");
      }

    } catch (Throwable e) {
      logger.error(e, e);
    }
  }
}

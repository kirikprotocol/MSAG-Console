package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.config.ConfigChanges;
import ru.novosoft.smsc.infosme.backend.config.tasks.Task;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataItem;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskFilter;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TasksTableHelper;

import javax.servlet.http.HttpServletRequest;
import java.util.Iterator;
import java.util.List;


/**
 * Created by igork
 * Date: Jul 31, 2003
 * Time: 2:07:44 PM
 */
public class Index extends IndexProperties
{
  public Index() {
    super(new TaskFilter(true));    
    setTableHelperMode(TasksTableHelper.MODE_ADMIN_ACTIVE, false);
  }

  protected int reset(HttpServletRequest req)
  {
    int result = RESULT_DONE;
    boolean options, tasks, schedules, providers, drivers, retries;
    options = tasks = schedules = providers = drivers = retries = false;

    if (isApply("all")) {
      options = tasks = schedules = providers = drivers = retries = true;
    }

    if (isApply("tasks")) {
      tasks = true;
    }

    if (isApply("scheds")) {
      schedules = true;
    }

    if (isApply("retries")) {
      retries = true;
    }

    try {
      String user = isUserAdmin(req) ? null : req.getRemoteUser();
      getInfoSmeConfig().reset(req.getRemoteUser(), user, options, tasks, schedules, retries, providers, drivers);
    } catch (Throwable e) {
      logger.error("Could not reload schedules", e);
      result = error("infosme.error.reload_schedules", e);
    }

    return result;
  }

  protected int apply(HttpServletRequest req)
  {
    try {
      logger.debug("Apply ...");
      boolean options, tasks, schedules, providers, drivers, retries;
      options = tasks = schedules = providers = drivers = retries = false;
      if (isApply("all")) {
        options = tasks = schedules = providers = drivers = retries = true;
      }
      if (isApply("tasks")) {
        tasks = true;
      }
      if (isApply("scheds")) {
        schedules = true;
      }
      if (isApply("retries")) {
        retries = true;
      }
      String user = isUserAdmin(req) ? null : req.getRemoteUser();

      ConfigChanges changes = getInfoSmeConfig().apply(req.getRemoteUser(), user, options, tasks, schedules, retries, providers, drivers);
      if (tasks) {
        // Notify InfoSme about new tasks
        for (Iterator iter = changes.getTasksChanges().getAdded().iterator(); iter.hasNext();)
          getInfoSme().addTask((String)iter.next());
        // Notify InfoSme about deleted tasks
        for (Iterator iter = changes.getTasksChanges().getDeleted().iterator(); iter.hasNext();)
          getInfoSme().removeTask((String)iter.next());
        // Notify InfoSme about archivated tasks
        for (Iterator iter = changes.getTasksChanges().getArchivated().iterator(); iter.hasNext();)
          getInfoSme().removeTask((String)iter.next());
        // Notify InfoSme about changed tasks
        for (Iterator iter = changes.getTasksChanges().getModified().iterator(); iter.hasNext();)
          getInfoSme().changeTask((String)iter.next());
      }

      if (schedules) {
        // Notify InfoSme about new schedules
        for (Iterator iter = changes.getSchedulesChanges().getAdded().iterator(); iter.hasNext();)
          getInfoSme().addSchedule((String)iter.next());
        // Notify InfoSme about deleted schedules
        for (Iterator iter = changes.getSchedulesChanges().getDeleted().iterator(); iter.hasNext();)
          getInfoSme().removeSchedule((String)iter.next());
        // Notify InfoSme about changed schedules
        for (Iterator iter = changes.getSchedulesChanges().getModified().iterator(); iter.hasNext();)
          getInfoSme().changeSchedule((String)iter.next());
      }

      if (retries)
        getInfoSme().applyRetryPolicies();

      if(options) {
        getInfoSme().reloadSmscAndRegions();
        return warning("infosme.prompt.restart");
      }

    } catch (Throwable e) {
      logger.error("Couldn't save InfoSME config", e);
      return error("infosme.error.config_save", e);
    }
    return RESULT_DONE;
  }

  protected int start()
  {
    int result = RESULT_DONE;
    if (isToStart("sme")) {
      try {
        getAppContext().getHostsManager().startService(getSmeId());
        try { // because of long start of InfoSme
          Thread.sleep(5000);
        } catch (InterruptedException e) {
          //do nothing
        }
      } catch (AdminException e) {
        logger.error("Could not start Info SME", e);
        result = error("infosme.error.start", e);
      }
    } else {
      if (isToStart("processor")) {
        try {
          getInfoSme().startTaskProcessor();
        } catch (AdminException e) {
          logger.error("Could not start task processor", e);
          result = error("infosme.error.start_tp", e);
        }
      }
      if (isToStart("scheduler")) {
        try {
          getInfoSme().startTaskScheduler();
        } catch (AdminException e) {
          logger.error("Could not start task scheduler", e);
          result = error("infosme.error.start_ts", e);
        }
      }
      if (isToStart("siebel")) {
        try {
          getInfoSmeContext().startSiebelTaskManager();
        } catch (AdminException e) {
          logger.error(e, e);
          result = error("infosme.error.start_siebel", e);
        }
      }
      if (isToStart("siebel")) {
        try {
          getInfoSmeContext().startSiebelTaskManager();
        } catch (AdminException e) {
          logger.error(e, e);
          result = error("infosme.error.start_siebel", e);
        }
      }
      if (isToStart("archiveDaemon")) {
        try {
          getInfoSmeContext().startArchiveDaemon();
        } catch (AdminException e) {
          logger.error(e, e);
          result = error("infosme.error.start_archive", e);
        }
      }
    }
    return result;
  }

  protected int stop()
  {
    int result = RESULT_DONE;
    if (isToStart("sme")) {
      try {
        getAppContext().getHostsManager().shutdownService(getSmeId());
        getInfoSmeContext().stopSiebelTaskManager();
      } catch (AdminException e) {
        logger.error("Could not stop Info SME", e);
        result = error("infosme.error.stop", e);
      }
    } else {
      if (isToStart("processor")) {
        try {
          getInfoSme().stopTaskProcessor();
        } catch (AdminException e) {
          logger.error("Could not stop task processor", e);
          result = error("infosme.error.stop_tp", e);
        }
      }
      if (isToStart("scheduler")) {
        try {
          getInfoSme().stopTaskScheduler();
        } catch (AdminException e) {
          logger.error("Could not stop task scheduler", e);
          result = error("infosme.error.stop_ts", e);
        }
      }
      if (isToStart("siebel")) {
        try {
          getInfoSmeContext().stopSiebelTaskManager();
        } catch (AdminException e) {
          logger.error("Could not stop siebel tm", e);
          result = error("infosme.error.stop_siebel", e);
        }
      }
      if (isToStart("archiveDaemon")) {
        try {
          getInfoSmeContext().stopTaskArchiveDaemon();
        } catch (AdminException e) {
          logger.error(e, e);
          result = error("infosme.error.stop_archive", e);
        }
      }
    }
    return result;
  }

  private int setTasksEnabled(boolean enabled)
  {
    int result = RESULT_DONE;
    List checked = getChecked();
    for (int i = 0; i < checked.size(); i++) {
      String taskId = (String)checked.get(i);
      try {
        Task t = getInfoSmeConfig().getTask(taskId);
        t.setEnabled(enabled);
        getInfoSmeConfig().addAndApplyTask(t);
        getInfoSme().setTaskEnabled(taskId, enabled);

      } catch (AdminException e) {
        logger.error("Could not enable task \"" + taskId + "\"", e);
        result = error("infosme.error.enable_task", taskId, e);
      }
    }
    return result;
  }

  protected int enableTask()
  {
    return setTasksEnabled(true);
  }

  protected int disableTask()
  {
    return setTasksEnabled(false);
  }

  protected int startTask()
  {
    int result = RESULT_DONE;
    try {
      getInfoSme().startTasks(getChecked());
    } catch (AdminException e) {
      logger.error("Could not start tasks", e);
      result = error("infosme.error.start_tasks", e);
    }
    return result;
  }

  protected int stopTask()
  {
    int result = RESULT_DONE;
    try {
      getInfoSme().stopTasks(getChecked());
    } catch (AdminException e) {
      logger.error("Could not stop tasks", e);
      result = error("infosme.error.stop_tasks", e);
    }
    return result;
  }

  public boolean isTaskEnabled(TaskDataItem task)
  {
    try {
      return isInfosmeStarted() ? getInfoSme().isTaskEnabled(task.getId()) : task.isEnabled();
    } catch (AdminException e) {
      logger.error("Could not get enabled status for task \"" + task.getId() + '"', e);
      return task.isEnabled();
    }
  }
}

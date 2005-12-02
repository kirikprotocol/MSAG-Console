package ru.novosoft.smsc.infosme.beans;

import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.Task;
import ru.novosoft.smsc.infosme.backend.schedules.Schedule;
import ru.novosoft.smsc.infosme.backend.tables.schedules.ScheduleDataSource;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataItem;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataSource;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.config.Config;

import javax.xml.parsers.ParserConfigurationException;
import java.io.IOException;
import java.text.ParseException;
import java.util.*;


/**
 * Created by igork
 * Date: Jul 31, 2003
 * Time: 2:07:44 PM
 */
public class Index extends IndexProperties
{
  protected int reset()
  {
    int result = RESULT_DONE;
    if (isApply("all")) {
      result = resetAll();
    }
    if (isApply("tasks")) {
      try {
        result = resetTasks();
      } catch (Throwable e) {
        logger.error("Could not reload tasks", e);
        result = error("Could not reload tasks", e);
      }
    }
    if (isApply("scheds")) {
      try {
        result = resetScheds();
      } catch (Throwable e) {
        logger.error("Could not reload schedules", e);
        result = error("Could not reload schedules", e);
      }
    }
    return result;
  }

  private int resetTasks() throws IOException, SAXException, ParserConfigurationException, AdminException, Config.ParamNotFoundException, Config.WrongParamTypeException
  {
    for (Iterator i = getConfig().getSectionChildShortSectionNames(TaskDataSource.TASKS_PREFIX).iterator(); i.hasNext();) {
      String taskId = (String) i.next();
      Task.removeTaskFromConfig(getConfig(), taskId);
    }
    Config oldConfig = getInfoSmeContext().loadCurrentConfig();
    for (Iterator i = oldConfig.getSectionChildShortSectionNames(TaskDataSource.TASKS_PREFIX).iterator(); i.hasNext();) {
      String taskId = (String) i.next();
      Task task = new Task(oldConfig, taskId);
      task.storeToConfig(getConfig());
    }
    getInfoSmeContext().setChangedTasks(false);
    return RESULT_DONE;
  }

  private int resetScheds() throws IOException, SAXException, ParserConfigurationException, AdminException, Config.ParamNotFoundException, Config.WrongParamTypeException, ParseException
  {
    for (Iterator i = getConfig().getSectionChildShortSectionNames(ScheduleDataSource.SCHEDULES_PREFIX).iterator(); i.hasNext();) {
      String schedId = (String) i.next();
      Schedule.removeScheduleFromConfig(schedId, getConfig());
    }
    Config oldConfig = getInfoSmeContext().loadCurrentConfig();
    for (Iterator i = oldConfig.getSectionChildShortSectionNames(ScheduleDataSource.SCHEDULES_PREFIX).iterator(); i.hasNext();) {
      String schedId = (String) i.next();
      Schedule schedule = Schedule.getInstance(schedId, oldConfig);
      schedule.storeToConfig(getConfig());
    }
    return RESULT_DONE;
  }

  private int resetAll()
  {
    try {
      getInfoSmeContext().resetConfig();
    } catch (Throwable e) {
      logger.debug("Couldn't reload InfoSME config", e);
      return error("Could not reload InfoSME config", e);
    }
    return RESULT_DONE;
  }

  protected int apply()
  {
    try {
      logger.debug("Apply ...");
      final Config oldConfig = getInfoSmeContext().loadCurrentConfig();
      if (isApply("all")) {
        return applyGlobalParams(oldConfig);
      } else if (isApply("tasks")) {
        if (getInfoSmeContext().isChangedDrivers() || getInfoSmeContext().isChangedOptions() || getInfoSmeContext().isChangedProviders())
          return error("You cannot apply tasks without applying global options");
        return applyTasks(oldConfig, getConfig());
      } else if (isApply("scheds")) {
        if (getInfoSmeContext().isChangedDrivers() || getInfoSmeContext().isChangedOptions() || getInfoSmeContext().isChangedProviders())
          return error("You cannot apply schedules without applying global options");
        if (getInfoSmeContext().isChangedTasks())
          return error("You cannot apply schedules without applying tasks");
        return applyScheds(oldConfig, getConfig());
      } else {

      }
    } catch (Throwable e) {
      logger.error("Couldn't save InfoSME config", e);
      return error("Could not save InfoSME config", e);
    }
    return RESULT_DONE;
  }

  private static final Set notGlobalSectionNames = new HashSet();
  static {
    notGlobalSectionNames.add("InfoSme.Schedules");
    notGlobalSectionNames.add("InfoSme.Tasks");
  }

  private int applyGlobalParams(final Config oldConfig) throws Config.WrongParamTypeException, IOException, NullPointerException, CloneNotSupportedException
  {
    Config backup = (Config) oldConfig.clone();
    final Config config = getConfig();

    //InfoSme - root
    backup.removeParamsFromSection("InfoSme");
    backup.copySectionParamsFromConfig(config, "InfoSme");

    //StartupLoader
    backup.removeSection("StartupLoader");
    backup.copySectionFromConfig(config, "StartupLoader");

    //others
    for (Iterator i = backup.getSectionChildSectionNames("InfoSme").iterator(); i.hasNext();) {
      String sectionName = (String) i.next();
      if (!notGlobalSectionNames.contains(sectionName)) {
        backup.removeSection(sectionName);
      }
    }

    for (Iterator i = config.getSectionChildSectionNames("InfoSme").iterator(); i.hasNext();) {
      String sectionName = (String) i.next();
      if (!notGlobalSectionNames.contains(sectionName)) {
        backup.copySectionFromConfig(config, sectionName);
      }
    }

    backup.save();
    getInfoSmeContext().setChangedOptions(false);
    getInfoSmeContext().setChangedDrivers(false);
    getInfoSmeContext().setChangedProviders(false);
    getInfoSmeContext().reloadDataSource(oldConfig, config);
    if (getInfoSmeContext().getDataSource() == null)
      warning("Invalid JDBC parameters");
    return message("Changes saved, you need to restart InfoSme to apply changes");
  }

  private int applyScheds(Config oldConfig, Config newConfig)
  {
    int result = RESULT_DONE;
    try {
      logger.debug("applyScheds");
      Set oldSchedules = oldConfig.getSectionChildShortSectionNames(ScheduleDataSource.SCHEDULES_PREFIX);
      Set newSchedules = newConfig.getSectionChildShortSectionNames(ScheduleDataSource.SCHEDULES_PREFIX);
      Set toDelete = new HashSet(oldSchedules);
      toDelete.removeAll(newSchedules);
      Set toAdd = new HashSet(newSchedules);
      toAdd.removeAll(oldSchedules);
      Set toChange = new HashSet(oldSchedules);
      toChange.retainAll(newSchedules);

      for (Iterator i = new ArrayList(toChange).iterator(); i.hasNext();) {
        String schedId = (String) i.next();
        if (!scheduleChanged(schedId, oldConfig, newConfig))
          toChange.remove(schedId);
      }
      int r = RESULT_DONE;
      if (toAdd.size() > 0) r = addScheds(oldConfig, newConfig, toAdd);
      result = result == RESULT_DONE ? r : result;
      if (toDelete.size() > 0) r = deleteScheds(oldConfig, toDelete);
      result = result == RESULT_DONE ? r : result;
      if (toChange.size() > 0) r = changeScheds(oldConfig, newConfig, toChange);
      result = result == RESULT_DONE ? r : result;

      getInfoSmeContext().setChangedSchedules(false);
      return result;
    } catch (AdminException e) {
      logger.error("Could not apply schedules", e);
      return error("Could not apply schedules", e);
    } catch (Throwable e) {
      logger.error("Could not apply schedules", e);
      return error("Could not apply schedules", e);
    }
  }

  private int changeScheds(Config oldConfig, Config newConfig, Set toChange) throws AdminException, Config.ParamNotFoundException, ParseException, Config.WrongParamTypeException, IOException
  {
    int result = RESULT_DONE;
    for (Iterator i = toChange.iterator(); i.hasNext();) {
      String schedId = (String) i.next();
      Schedule schedule = Schedule.getInstance(schedId, newConfig);
      try {
        Config backup = (Config) oldConfig.clone();
        schedule.storeToConfig(oldConfig);
        oldConfig.save();
        if (getInfoSme().getInfo().isOnline()) {
          try {
            getInfoSmeContext().getInfoSme().changeSchedule(schedId);
          } catch (AdminException e) {
            logger.error("Could not change schedule \"" + schedId + '"', e);
            result = error("Could not change schedule", schedId, e);
            backup.save();
          }
        }
      } catch (CloneNotSupportedException e) {
        logger.fatal("Internal error", e);
      }
    }
    return result;
  }

  private int deleteScheds(Config oldConfig, Set toDelete) throws IOException, Config.WrongParamTypeException
  {
    int result = RESULT_DONE;
    for (Iterator i = toDelete.iterator(); i.hasNext();) {
      String schedId = (String) i.next();
      try {
        Config backup = (Config) oldConfig.clone();
        Schedule.removeScheduleFromConfig(schedId, oldConfig);
        oldConfig.save();
        if (getInfoSme().getInfo().isOnline()) {
          try {
            getInfoSmeContext().getInfoSme().removeSchedule(schedId);
          } catch (AdminException e) {
            logger.error("Could not delete schedule \"" + schedId + '"', e);
            result = error("Could not delete schedule", schedId, e);
            backup.save();
          }
        }
      } catch (CloneNotSupportedException e) {
        logger.fatal("Internal error", e);
      }
    }
    return result;
  }

  private int addScheds(Config oldConfig, Config newConfig, Collection toAdd) throws AdminException, Config.ParamNotFoundException, ParseException, Config.WrongParamTypeException, IOException
  {
    int result = RESULT_DONE;
    for (Iterator i = toAdd.iterator(); i.hasNext();) {
      String schedId = (String) i.next();
      Schedule schedule = Schedule.getInstance(schedId, newConfig);
      try {
        Config backup = (Config) oldConfig.clone();
        schedule.storeToConfig(oldConfig);
        oldConfig.save();
        if (getInfoSme().getInfo().isOnline()) {
          try {
            getInfoSmeContext().getInfoSme().addSchedule(schedId);
          } catch (AdminException e) {
            logger.error("Could not add schedule \"" + schedId + '"', e);
            result = error("Could not add schedule", schedId, e);
            backup.save();
          }
        }
      } catch (CloneNotSupportedException e) {
        logger.fatal("Internal error", e);
      }
    }
    return result;
  }

  private boolean scheduleChanged(String schedId, Config oldConfig, Config newConfig) throws AdminException, Config.ParamNotFoundException, ParseException, Config.WrongParamTypeException
  {
    Schedule oldSchedule = Schedule.getInstance(schedId, oldConfig);
    Schedule newSchedule = Schedule.getInstance(schedId, newConfig);
    return !oldSchedule.equals(newSchedule);
  }

  private int applyTasks(Config oldConfig, Config newConfig)
  {
    int result = RESULT_DONE;
    try {
      logger.debug("applyTasks");
      Set oldTasks = oldConfig.getSectionChildShortSectionNames(TaskDataSource.TASKS_PREFIX);
      Set newTasks = newConfig.getSectionChildShortSectionNames(TaskDataSource.TASKS_PREFIX);
      Set toDelete = new HashSet(oldTasks);
      toDelete.removeAll(newTasks);
      Set toAdd = new HashSet(newTasks);
      toAdd.removeAll(oldTasks);
      Set toChange = new HashSet(oldTasks);
      toChange.retainAll(newTasks);

      for (Iterator i = new ArrayList(toChange).iterator(); i.hasNext();) {
        String taskId = (String) i.next();
        if (!taskChanged(taskId, oldConfig, newConfig))
          toChange.remove(taskId);
      }

      int r = RESULT_DONE;
      if (toAdd.size() > 0) r = addTasks(oldConfig, newConfig, toAdd);
      result = result == RESULT_DONE ? r : result;
      if (toDelete.size() > 0) r = deleteTasks(oldConfig, toDelete);
      result = result == RESULT_DONE ? r : result;
      if (toChange.size() > 0) r = changeTasks(oldConfig, newConfig, toChange);
      result = result == RESULT_DONE ? r : result;

      getInfoSmeContext().setChangedTasks(false);
      if (result == RESULT_DONE)
        return applyScheds(oldConfig, newConfig);
      else
        return warning("Not all tasks applied properly, so schedules not tried to apply.");
    } catch (AdminException e) {
      logger.error("Could not apply tasks", e);
      return error("Could not apply tasks", e);
    } catch (Throwable e) {
      logger.error("Could not apply tasks", e);
      return error("Could not apply tasks", e);
    }
  }

  private int changeTasks(Config oldConfig, Config newConfig, Collection toChange) throws Config.WrongParamTypeException, Config.ParamNotFoundException, IOException
  {
    int result = RESULT_DONE;
    for (Iterator i = toChange.iterator(); i.hasNext();) {
      String taskId = (String) i.next();
      Task task = new Task(newConfig, taskId);
      try {
        Config backup = (Config) oldConfig.clone();
        task.storeToConfig(oldConfig);
        oldConfig.save();
        if (getInfoSme().getInfo().isOnline()) {
          try {
            getInfoSme().changeTask(taskId);
          } catch (AdminException e) {
            logger.error("Could not change task \"" + taskId + '"', e);
            result = error("Could not change task", taskId, e);
            backup.save();
          }
        }
      } catch (CloneNotSupportedException e) {
        logger.fatal("Internal error", e);
      }
    }
    return result;
  }

  private int deleteTasks(Config config, Collection toDelete) throws Config.WrongParamTypeException, IOException
  {
    int result = RESULT_DONE;
    for (Iterator i = toDelete.iterator(); i.hasNext();) {
      String taskId = (String) i.next();
      try {
        Config backup = (Config) config.clone();
        Task.removeTaskFromConfig(config, taskId);
        config.save();
        if (getInfoSme().getInfo().isOnline()) {
          try {
            getInfoSmeContext().getInfoSme().removeTask(taskId);
          } catch (AdminException e) {
            logger.error("Could not delete task \"" + taskId + '"', e);
            result = error("Could not delete task", taskId, e);
            backup.save();
          }
        }
      } catch (CloneNotSupportedException e) {
        logger.fatal("Internal error", e);
      }
    }
    return result;
  }

  private int addTasks(Config oldConfig, Config newConfig, Collection toAdd) throws AdminException, Config.WrongParamTypeException, IOException, Config.ParamNotFoundException
  {
    int result = RESULT_DONE;
    for (Iterator i = toAdd.iterator(); i.hasNext();) {
      String taskId = (String) i.next();
      Task task = new Task(newConfig, taskId);
      try {
        Config backup = (Config) oldConfig.clone();
        task.storeToConfig(oldConfig);
        oldConfig.save();
        if (getInfoSme().getInfo().isOnline()) {
          try {
            getInfoSmeContext().getInfoSme().addTask(taskId);
          } catch (AdminException e) {
            logger.error("Could not add task \"" + taskId + '"', e);
            result = error("Could not add task", taskId, e);
            backup.save();
            throw e;
          }
        }
      } catch (CloneNotSupportedException e) {
        logger.fatal("Internal error", e);
      }
    }
    return result;
  }

  private boolean taskChanged(String taskId, Config oldConfig, Config newConfig) throws Config.WrongParamTypeException, Config.ParamNotFoundException
  {
    Task oldTask = new Task(oldConfig, taskId);
    Task newTask = new Task(newConfig, taskId);
    return !oldTask.equals(newTask);
  }

  protected int start()
  {
    int result = RESULT_DONE;
    if (isToStart("sme")) {
      try {
        getAppContext().getHostsManager().startService(getSmeId());
        try { // из-за долгого старта InfoSme
          Thread.sleep(5000);
        } catch (InterruptedException e) {
          //do nothing
        }
      } catch (AdminException e) {
        logger.error("Could not start Info SME", e);
        result = error("Could not start Info SME", e);
      }
      return RESULT_DONE;
    } else {
      if (isToStart("processor")) {
        try {
          getInfoSme().startTaskProcessor();
        } catch (AdminException e) {
          logger.error("Could not start task processor", e);
          result = error("Could not start task processor", e);
        }
      }
      if (isToStart("scheduler")) {
        try {
          getInfoSme().startTaskScheduler();
        } catch (AdminException e) {
          logger.error("Could not start task scheduler", e);
          result = error("Could not start task scheduler", e);
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
      } catch (AdminException e) {
        logger.error("Could not stop Info SME", e);
        result = error("Could not stop Info SME", e);
      }
      return RESULT_DONE;
    } else {
      if (isToStart("processor")) {
        try {
          getInfoSme().stopTaskProcessor();
        } catch (AdminException e) {
          logger.error("Could not stop task processor", e);
          result = error("Could not stop task processor", e);
        }
      }
      if (isToStart("scheduler")) {
        try {
          getInfoSme().stopTaskScheduler();
        } catch (AdminException e) {
          logger.error("Could not stop task scheduler", e);
          result = error("Could not stop task scheduler", e);
        }
      }
    }
    return result;
  }

  private int setTasksEnabled(boolean enabled)
  {
    int result = RESULT_DONE;
    Config currentConfig = null;
    try {
      currentConfig = getInfoSmeContext().loadCurrentConfig();
    } catch (Throwable e) {
      logger.error("Could not load current config", e);
    }
    for (int i = 0; i < getChecked().length; i++) {
      String taskId = getChecked()[i];
      final String prefix = TaskDataSource.TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(taskId);
      try {
        getInfoSme().setTaskEnabled(taskId, enabled);
        if (currentConfig != null) currentConfig.setBool(prefix + ".enabled", enabled);
        if (getConfig().containsSection(prefix)) getConfig().setBool(prefix + ".enabled", enabled);
      } catch (AdminException e) {
        logger.error("Could not enable task \"" + taskId + "\"", e);
        result = error("Could not enable task", taskId, e);
      }
    }
    try {
      if (currentConfig != null) currentConfig.save();
    } catch (Throwable e) {
      logger.error("Could not save current config", e);
      result = error("Could not save current config", e);
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
      getInfoSme().startTasks(getCheckedSet());
    } catch (AdminException e) {
      logger.error("Could not start tasks", e);
      result = error("Could not start tasks", e);
    }
    return result;
  }

  protected int stopTask()
  {
    int result = RESULT_DONE;
    try {
      getInfoSme().stopTasks(getCheckedSet());
    } catch (AdminException e) {
      logger.error("Could not stop tasks", e);
      result = error("Could not stop tasks", e);
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

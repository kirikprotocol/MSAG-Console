package ru.novosoft.smsc.infosme.beans;

import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.service.ServiceInfo;
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
        getConfig().save();
        getInfoSmeContext().setChangedOptions(false);
        getInfoSmeContext().setChangedDrivers(false);
        getInfoSmeContext().setChangedProviders(false);
        getInfoSmeContext().setChangedSchedules(false);
        getInfoSmeContext().setChangedTasks(false);
        getInfoSmeContext().reloadDataSource(oldConfig, getConfig());
        if (getInfoSmeContext().getDataSource() == null)
          warning("Invalid JDBC parameters");
        return message("Changes saved, you need to restart InfoSme to apply changes");
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

  private int applyScheds(Config oldConfig, Config newConfig)
  {
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
      if (toAdd.size() > 0) addScheds(oldConfig, newConfig, toAdd);
      if (toDelete.size() > 0) deleteScheds(oldConfig, toDelete);
      if (toChange.size() > 0) changeScheds(oldConfig, newConfig, toChange);
      getInfoSmeContext().setChangedSchedules(false);
    } catch (Throwable e) {
      logger.error("Could not apply schedules", e);
      return error("Could not apply schedules", e);
    }
    return RESULT_DONE;
  }

  private void changeScheds(Config oldConfig, Config newConfig, Set toChange) throws AdminException, Config.ParamNotFoundException, ParseException, Config.WrongParamTypeException, IOException
  {
    for (Iterator i = toChange.iterator(); i.hasNext();) {
      String schedId = (String) i.next();
      Schedule schedule = Schedule.getInstance(schedId, newConfig);
      schedule.storeToConfig(oldConfig);
    }
    oldConfig.save();
    if (getInfoSme().getInfo().getStatus() == ServiceInfo.STATUS_RUNNING)
      getInfoSmeContext().getInfoSme().changeSchedules(toChange);
  }

  private void deleteScheds(Config oldConfig, Set toDelete) throws AdminException, IOException, Config.WrongParamTypeException
  {
    for (Iterator i = toDelete.iterator(); i.hasNext();) {
      String schedId = (String) i.next();
      Schedule.removeScheduleFromConfig(schedId, oldConfig);
    }
    oldConfig.save();
    if (getInfoSme().getInfo().getStatus() == ServiceInfo.STATUS_RUNNING)
      getInfoSmeContext().getInfoSme().removeSchedules(toDelete);
  }

  private void addScheds(Config oldConfig, Config newConfig, Collection toAdd) throws AdminException, Config.ParamNotFoundException, ParseException, Config.WrongParamTypeException, IOException
  {
    for (Iterator i = toAdd.iterator(); i.hasNext();) {
      String schedId = (String) i.next();
      Schedule schedule = Schedule.getInstance(schedId, newConfig);
      schedule.storeToConfig(oldConfig);
    }
    oldConfig.save();
    if (getInfoSme().getInfo().getStatus() == ServiceInfo.STATUS_RUNNING)
      getInfoSmeContext().getInfoSme().addSchedules(toAdd);
  }

  private boolean scheduleChanged(String schedId, Config oldConfig, Config newConfig) throws AdminException, Config.ParamNotFoundException, ParseException, Config.WrongParamTypeException
  {
    Schedule oldSchedule = Schedule.getInstance(schedId, oldConfig);
    Schedule newSchedule = Schedule.getInstance(schedId, newConfig);
    return !oldSchedule.equals(newSchedule);
  }

  private int applyTasks(Config oldConfig, Config newConfig)
  {
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

      if (toAdd.size() > 0) addTasks(oldConfig, newConfig, toAdd);
      if (toDelete.size() > 0) deleteTasks(oldConfig, toDelete);
      if (toChange.size() > 0) changeTasks(oldConfig, newConfig, toChange);

      getInfoSmeContext().setChangedTasks(false);
      applyScheds(oldConfig, newConfig);
    } catch (Throwable e) {
      logger.error("Could not apply tasks", e);
      return error("Could not apply tasks", e);
    }
    return RESULT_DONE;
  }

  private void changeTasks(Config oldConfig, Config newConfig, Collection toChange) throws AdminException, Config.WrongParamTypeException, Config.ParamNotFoundException, IOException
  {
    for (Iterator i = toChange.iterator(); i.hasNext();) {
      String taskId = (String) i.next();
      Task task = new Task(newConfig, taskId);
      task.storeToConfig(oldConfig);
    }
    oldConfig.save();
    if (getInfoSme().getInfo().getStatus() == ServiceInfo.STATUS_RUNNING)
      getInfoSme().changeTasks(toChange);
  }

  private void deleteTasks(Config config, Collection toDelete) throws AdminException, Config.WrongParamTypeException, IOException
  {
    for (Iterator i = toDelete.iterator(); i.hasNext();) {
      String taskId = (String) i.next();
      Task.removeTaskFromConfig(config, taskId);
    }
    config.save();
    if (getInfoSme().getInfo().getStatus() == ServiceInfo.STATUS_RUNNING)
      getInfoSmeContext().getInfoSme().removeTasks(toDelete);
  }

  private void addTasks(Config oldConfig, Config newConfig, Collection toAdd) throws AdminException, Config.WrongParamTypeException, IOException, Config.ParamNotFoundException
  {
    for (Iterator i = toAdd.iterator(); i.hasNext();) {
      String taskId = (String) i.next();
      Task task = new Task(newConfig, taskId);
      task.storeToConfig(oldConfig);
    }
    oldConfig.save();
    if (getInfoSme().getInfo().getStatus() == ServiceInfo.STATUS_RUNNING)
      getInfoSmeContext().getInfoSme().addTasks(toAdd);
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
        getAppContext().getHostsManager().startService(Constants.INFO_SME_ID);
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
        getAppContext().getHostsManager().shutdownService(Constants.INFO_SME_ID);
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

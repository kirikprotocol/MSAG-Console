package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.infosme.backend.tables.schedules.ScheduleDataSource;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataSource;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.config.Config;

import java.security.Principal;
import java.util.*;


/**
 * Created by igork
 * Date: Jul 31, 2003
 * Time: 2:07:44 PM
 */
public class Index extends InfoSmeBean
{
  private String mbApply = null;
  private String mbResetAll = null;
  private String[] apply = new String[0];

  public int process(SMSCAppContext appContext, List errors, Principal loginedPrincipal)
  {
    int result = super.process(appContext, errors, loginedPrincipal);
    if (result != RESULT_OK)
      return result;

    if (mbApply != null)
      return apply();
    if (mbResetAll != null)
      return resetAll();

    return result;
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

  private int apply()
  {
    try {
      logger.debug("Apply ...");
      final Config oldConfig = getInfoSmeContext().loadCurrentConfig();
      Set applySet = new HashSet(Arrays.asList(apply));
      if (applySet.contains("all")) {
        getConfig().save();
        getInfoSmeContext().setChangedOptions(false);
        getInfoSmeContext().setChangedDrivers(false);
        getInfoSmeContext().setChangedProviders(false);
        getInfoSmeContext().setChangedSchedules(false);
        getInfoSmeContext().setChangedTasks(false);
        return message("Changes saved, you need to restart InfoSme to apply changes");
      } else if (applySet.contains("tasks")) {
        if (getInfoSmeContext().isChangedDrivers() || getInfoSmeContext().isChangedOptions() || getInfoSmeContext().isChangedProviders())
          return error("You cannot apply tasks without applying global options");
        getConfig().save();
        return applyTasks(oldConfig, getConfig());
      } else if (applySet.contains("scheds")) {
        if (getInfoSmeContext().isChangedDrivers() || getInfoSmeContext().isChangedOptions() || getInfoSmeContext().isChangedProviders())
          return error("You cannot apply schedules without applying global options");
        if (getInfoSmeContext().isChangedTasks())
          return error("You cannot apply schedules without applying tasks");
        getConfig().save();
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

      for (Iterator i = toAdd.iterator(); i.hasNext();) {
        String schedId = (String) i.next();
        logger.debug("add sched \"" + schedId + '"');
        getInfoSmeContext().getInfoSme().addSchedule(schedId);
      }
      logger.debug("toChange: " + toChange.size());
      for (Iterator i = toChange.iterator(); i.hasNext();) {
        String schedId = (String) i.next();
        if (scheduleChanged(schedId, oldConfig, newConfig)) {
          logger.debug("change sched \"" + schedId + '"');
          getInfoSmeContext().getInfoSme().changeSchedule(schedId, schedId);
        }
      }
      for (Iterator i = toDelete.iterator(); i.hasNext();) {
        String schedId = (String) i.next();
        logger.debug("del sched \"" + schedId + '"');
        getInfoSmeContext().getInfoSme().removeSchedule(schedId);
      }
      getInfoSmeContext().setChangedSchedules(false);
    } catch (Throwable e) {
      logger.error("Could not apply schedules", e);
      return error("Could not apply schedules", e);
    }
    return RESULT_DONE;
  }

  private boolean scheduleChanged(String schedId, Config oldConfig, Config config)
  {
    try {
      logger.debug("isScheduleChanged \"" + schedId + '"');
      final String prefix = ScheduleDataSource.SCHEDULES_PREFIX + '.' + StringEncoderDecoder.encodeDot(schedId);
      boolean equals = config.getString(prefix + ".execute").equals(oldConfig.getString(prefix + ".execute"))
              && config.getString(prefix + ".tasks").equals(oldConfig.getString(prefix + ".tasks"))
              && config.getString(prefix + ".startDateTime").equals(oldConfig.getString(prefix + ".startDateTime"));
      if (equals) {
        String execute = config.getString(prefix + ".execute");
        if ("once".equalsIgnoreCase(execute)) {
        } else if ("daily".equalsIgnoreCase(execute)) {
          equals &= config.getString(prefix + ".endDateTime").equals(oldConfig.getString(prefix + ".endDateTime"));
          equals &= config.getInt(prefix + ".everyNDays") == oldConfig.getInt(prefix + ".everyNDays");
        } else if ("weekly".equalsIgnoreCase(execute)) {
          equals &= config.getString(prefix + ".endDateTime").equals(oldConfig.getString(prefix + ".endDateTime"));
          equals &= config.getInt(prefix + ".everyNWeeks") == oldConfig.getInt(prefix + ".everyNWeeks");
          equals &= config.getString(prefix + ".weekDays").equals(oldConfig.getString(prefix + ".weekDays"));
        } else if ("monthly".equalsIgnoreCase(execute)) {
          equals &= config.getString(prefix + ".endDateTime").equals(oldConfig.getString(prefix + ".endDateTime"));
          if (config.containsParameter(prefix + ".dayOfMonth")) {
            equals &= config.getInt(prefix + ".dayOfMonth") == oldConfig.getInt(prefix + ".dayOfMonth");
          } else {
            equals &= config.getString(prefix + ".weekDayN").equals(oldConfig.getString(prefix + ".weekDayN"));
            equals &= config.getString(prefix + ".weekDay").equals(oldConfig.getString(prefix + ".weekDay"));
          }
          equals &= config.getString(prefix + ".monthes").equals(oldConfig.getString(prefix + ".monthes"));
        } else if ("interval".equalsIgnoreCase(execute)) {
          equals &= config.getString(prefix + ".endDateTime").equals(oldConfig.getString(prefix + ".endDateTime"));
          equals &= config.getString(prefix + ".intervalTime").equals(oldConfig.getString(prefix + ".intervalTime"));
        } else {
          logger.error("Unknown type of schedule: \"" + execute + "\"");
          return true;
        }
      }
      logger.debug("isScheduleChanged \"" + schedId + "\" : " + !equals);
      return !equals;
    } catch (Exception e) {
      logger.error(e);
      return true;
    }

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

      for (Iterator i = toChange.iterator(); i.hasNext();) {
        String taskId = (String) i.next();
        if (taskChanged(taskId, oldConfig, newConfig)) {
          toDelete.add(taskId);
          toAdd.add(taskId);
        }
      }

      getInfoSmeContext().getInfoSme().addTasks(toAdd);
      getInfoSmeContext().getInfoSme().removeTasks(toDelete);
      getInfoSmeContext().setChangedTasks(false);
      applyScheds(oldConfig, newConfig);
    } catch (Throwable e) {
      logger.error("Could not apply tasks", e);
      return error("Could not apply tasks", e);
    }
    return RESULT_DONE;
  }

  private boolean taskChanged(String taskId, Config oldConfig, Config newConfig) throws Config.WrongParamTypeException, Config.ParamNotFoundException
  {
    final String prefix = TaskDataSource.TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(taskId);
    return !(oldConfig.getString(prefix + ".dsId").equals(newConfig.getString(prefix + ".dsId"))
            && oldConfig.getBool(prefix + ".enabled") == newConfig.getBool(prefix + ".enabled")
            && oldConfig.getInt(prefix + ".priority") == newConfig.getInt(prefix + ".priority")
            && oldConfig.getBool(prefix + ".retryOnFail") == newConfig.getBool(prefix + ".retryOnFail")
            && oldConfig.getBool(prefix + ".replaceMessage") == newConfig.getBool(prefix + ".replaceMessage")
            && oldConfig.getString(prefix + ".svcType").equals(newConfig.getString(prefix + ".svcType"))
            && oldConfig.getString(prefix + ".endDate").equals(newConfig.getString(prefix + ".endDate"))
            && oldConfig.getString(prefix + ".retryTime").equals(newConfig.getString(prefix + ".retryTime"))
            && oldConfig.getString(prefix + ".validityPeriod").equals(newConfig.getString(prefix + ".validityPeriod"))
            && oldConfig.getString(prefix + ".validityDate").equals(newConfig.getString(prefix + ".validityDate"))
            && oldConfig.getString(prefix + ".activePeriodStart").equals(newConfig.getString(prefix + ".activePeriodStart"))
            && oldConfig.getString(prefix + ".activePeriodEnd").equals(newConfig.getString(prefix + ".activePeriodEnd"))
            && oldConfig.getString(prefix + ".query").equals(newConfig.getString(prefix + ".query"))
            && oldConfig.getString(prefix + ".template").equals(newConfig.getString(prefix + ".template"))
            && oldConfig.getInt(prefix + ".dsOwnTimeout") == newConfig.getInt(prefix + ".dsOwnTimeout")
            && oldConfig.getInt(prefix + ".dsIntTimeout") == newConfig.getInt(prefix + ".dsIntTimeout")
            && oldConfig.getInt(prefix + ".messagesCacheSize") == newConfig.getInt(prefix + ".messagesCacheSize")
            && oldConfig.getInt(prefix + ".messagesCacheSleep") == newConfig.getInt(prefix + ".messagesCacheSleep")
            && oldConfig.getBool(prefix + ".transactionMode") == newConfig.getBool(prefix + ".transactionMode")
            && oldConfig.getInt(prefix + ".uncommitedInGeneration") == newConfig.getInt(prefix + ".uncommitedInGeneration")
            && oldConfig.getInt(prefix + ".uncommitedInProcess") == newConfig.getInt(prefix + ".uncommitedInProcess"));
  }

  public String getMbApply()
  {
    return mbApply;
  }

  public void setMbApply(String mbApply)
  {
    this.mbApply = mbApply;
  }

  public String getMbResetAll()
  {
    return mbResetAll;
  }

  public void setMbResetAll(String mbResetAll)
  {
    this.mbResetAll = mbResetAll;
  }

  public boolean isChangedAll()
  {
    return getInfoSmeContext().isChangedOptions() || getInfoSmeContext().isChangedDrivers() || getInfoSmeContext().isChangedProviders();
  }

  public boolean isChangedTasks()
  {
    return getInfoSmeContext().isChangedTasks();
  }

  public boolean isChangedShedules()
  {
    return getInfoSmeContext().isChangedSchedules();
  }

  public String[] getApply()
  {
    return apply;
  }

  public void setApply(String[] apply)
  {
    this.apply = apply;
  }
}

package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.infosme.backend.tables.schedules.ScheduleDataSource;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataSource;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.security.Principal;
import java.util.*;

/**
 * Created by igork
 * Date: Sep 2, 2003
 * Time: 2:31:58 PM
 */
public class ScheduleEdit extends InfoSmeBean
{
  private String mbDone = null;
  private String mbCancel = null;

  private boolean initialized = false;
  private boolean create = false;
  private String oldSchedule = null;

  private String name = null;
  private String execute = null;
  private String startDateTime = null;
  private String endDateTime = null;
  private int everyNDays = 0;
  private int everyNWeeks = 0;
  private int dayOfMonth = 0;
  private String weekDayN = null;
  private String weekDay = null;
  private String intervalTime = null;

  private String[] checkedMonths = new String[0];
  private Collection checkedMonthsSet = new HashSet();
  private String monthlyType = null;
  private String[] checkedWeekDays = new String[0];
  private Collection checkedWeekDaysSet = new HashSet();
  private String[] checkedTasks = new String[0];
  private Collection checkedTasksSet = new HashSet();

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (!initialized) {
      if (!create) {
        if (name == null || name.length() == 0)
          return error("Schedule not specified");

        try {
          final String prefix = ScheduleDataSource.SCHEDULES_PREFIX + '.' + StringEncoderDecoder.encodeDot(name);
          execute = getConfig().getString(prefix + ".execute");
          Functions.addValuesToCollection(checkedTasksSet, getConfig().getString(prefix + ".tasks"), ",", true);
          startDateTime = getConfig().getString(prefix + ".startDateTime");
          if ("once".equalsIgnoreCase(execute)) {
          } else if ("daily".equalsIgnoreCase(execute)) {
            endDateTime = getConfig().getString(prefix + ".endDateTime");
            everyNDays = getConfig().getInt(prefix + ".everyNDays");
          } else if ("weekly".equalsIgnoreCase(execute)) {
            endDateTime = getConfig().getString(prefix + ".endDateTime");
            everyNWeeks = getConfig().getInt(prefix + ".everyNWeeks");
            Functions.addValuesToCollection(checkedWeekDaysSet, getConfig().getString(prefix + ".weekDays"), ",", true);
          } else if ("monthly".equalsIgnoreCase(execute)) {
            endDateTime = getConfig().getString(prefix + ".endDateTime");
            if (getConfig().containsParameter(prefix + ".dayOfMonth")) {
              monthlyType = "day";
              dayOfMonth = getConfig().getInt(prefix + ".dayOfMonth");
            } else {
              monthlyType = "week";
              weekDayN = getConfig().getString(prefix + ".weekDayN");
              weekDay = getConfig().getString(prefix + ".weekDay");
            }
            Functions.addValuesToCollection(checkedMonthsSet, getConfig().getString(prefix + ".monthes"), ",", true);
          } else if ("interval".equalsIgnoreCase(execute)) {
            endDateTime = getConfig().getString(prefix + ".endDateTime");
            intervalTime = getConfig().getString(prefix + ".intervalTime");
          } else {
            return error("Unknown type of schedule: \"" + execute + "\"");
          }
          oldSchedule = name;
        } catch (Exception e) {
          logger.error(e);
          return error(e.getMessage());
        } finally {
          if (name == null) name = "";
          if (execute == null) execute = "";
          if (startDateTime == null) startDateTime = "";
          if (endDateTime == null) endDateTime = "";
          if (weekDayN == null) weekDayN = "";
          if (weekDay == null) weekDay = "";
          if (intervalTime == null) intervalTime = "";
          if (oldSchedule == null) oldSchedule = "";
          if (monthlyType == null) monthlyType = "";
        }
      }
    }
    if (name == null) name = "";
    if (execute == null) execute = "";
    if (startDateTime == null) startDateTime = "";
    if (endDateTime == null) endDateTime = "";
    if (weekDayN == null) weekDayN = "";
    if (weekDay == null) weekDay = "";
    if (intervalTime == null) intervalTime = "";
    if (oldSchedule == null) oldSchedule = "";
    if (monthlyType == null) monthlyType = "";

    checkedMonthsSet.addAll(Arrays.asList(checkedMonths));
    checkedWeekDaysSet.addAll(Arrays.asList(checkedWeekDays));
    checkedTasksSet.addAll(Arrays.asList(checkedTasks));

    return result;
  }

  public int process(SMSCAppContext appContext, List errors, Principal loginedPrincipal)
  {
    int result = super.process(appContext, errors, loginedPrincipal);
    if (result != RESULT_OK)
      return result;

    if (mbDone != null)
      return done();
    if (mbCancel != null)
      return RESULT_DONE;

    return result;
  }

  protected int done()
  {
    if (name == null || name.length() == 0)
      return error("Schedule name not specified");
    final String prefix = ScheduleDataSource.SCHEDULES_PREFIX + '.' + StringEncoderDecoder.encodeDot(name);
    if (create) {
      if (getConfig().containsSection(prefix))
        return error("Schedule already exists", name);
    } else {
      if (!oldSchedule.equals(name)) {
        if (getConfig().containsSection(prefix))
          return error("Schedule already exists", name);
      }
      getConfig().removeSection(ScheduleDataSource.SCHEDULES_PREFIX + '.' + StringEncoderDecoder.encodeDot(oldSchedule));
    }

    if ("once".equalsIgnoreCase(execute)) {
    } else if ("daily".equalsIgnoreCase(execute)) {
      getConfig().setString(prefix + ".endDateTime", endDateTime);
      getConfig().setInt(prefix + ".everyNDays", everyNDays);
    } else if ("weekly".equalsIgnoreCase(execute)) {
      getConfig().setString(prefix + ".endDateTime", endDateTime);
      getConfig().setInt(prefix + ".everyNWeeks", everyNWeeks);
      getConfig().setString(prefix + ".weekDays", Functions.collectionToString(checkedWeekDaysSet, ","));
    } else if ("monthly".equalsIgnoreCase(execute)) {
      getConfig().setString(prefix + ".endDateTime", endDateTime);
      if (monthlyType.equals("day")) {
        getConfig().setInt(prefix + ".dayOfMonth", dayOfMonth);
      } else {
        getConfig().setString(prefix + ".weekDayN", weekDayN);
        getConfig().setString(prefix + ".weekDay", weekDay);
      }
      getConfig().setString(prefix + ".monthes", Functions.collectionToString(checkedMonthsSet, ","));
    } else if ("interval".equalsIgnoreCase(execute)) {
      getConfig().setString(prefix + ".endDateTime", endDateTime);
      getConfig().setString(prefix + ".intervalTime", intervalTime);
    } else {
      return error("Unknown type of schedule: \"" + execute + "\"");
    }
    getConfig().setString(prefix + ".execute", execute);
    getConfig().setString(prefix + ".tasks", Functions.collectionToString(checkedTasksSet, ","));
    getConfig().setString(prefix + ".startDateTime", startDateTime);
    getInfoSmeContext().setChangedSchedules(true);
    return RESULT_DONE;
  }

  public boolean isInitialized()
  {
    return initialized;
  }

  public void setInitialized(boolean initialized)
  {
    this.initialized = initialized;
  }

  public boolean isCreate()
  {
    return create;
  }

  public void setCreate(boolean create)
  {
    this.create = create;
  }

  public String getName()
  {
    return name;
  }

  public void setName(String name)
  {
    this.name = name;
  }

  public String getMbDone()
  {
    return mbDone;
  }

  public void setMbDone(String mbDone)
  {
    this.mbDone = mbDone;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }

  public String getOldSchedule()
  {
    return oldSchedule;
  }

  public void setOldSchedule(String oldSchedule)
  {
    this.oldSchedule = oldSchedule;
  }

  public String getExecute()
  {
    return execute;
  }

  public void setExecute(String execute)
  {
    this.execute = execute;
  }

  public String getStartDateTime()
  {
    return startDateTime;
  }

  public void setStartDateTime(String startDateTime)
  {
    this.startDateTime = startDateTime;
  }

  public String getEndDateTime()
  {
    return endDateTime;
  }

  public void setEndDateTime(String endDateTime)
  {
    this.endDateTime = endDateTime;
  }

  public int getEveryNDaysInt()
  {
    return everyNDays;
  }

  public void setEveryNDaysInt(int everyNDays)
  {
    this.everyNDays = everyNDays;
  }

  public String getEveryNDays()
  {
    return String.valueOf(everyNDays);
  }

  public void setEveryNDays(String everyNDays)
  {
    try {
      this.everyNDays = Integer.decode(everyNDays).intValue();
    } catch (Throwable e) {
      logger.error("Invalid value for everyNDays parameter: \"" + everyNDays + "\"", e);
    }
  }

  public int getEveryNWeeksInt()
  {
    return everyNWeeks;
  }

  public void setEveryNWeeksInt(int everyNWeeks)
  {
    this.everyNWeeks = everyNWeeks;
  }

  public String getEveryNWeeks()
  {
    return String.valueOf(everyNWeeks);
  }

  public void setEveryNWeeks(String everyNWeeks)
  {
    try {
      this.everyNWeeks = Integer.decode(everyNWeeks).intValue();
    } catch (Throwable e) {
      logger.error("Invalid value for everyNWeeks parameter: \"" + everyNWeeks + "\"", e);
    }
  }

  public int getDayOfMonthInt()
  {
    return dayOfMonth;
  }

  public void setDayOfMonthInt(int dayOfMonth)
  {
    this.dayOfMonth = dayOfMonth;
  }

  public String getDayOfMonth()
  {
    return String.valueOf(dayOfMonth);
  }

  public void setDayOfMonth(String dayOfMonth)
  {
    try {
      this.dayOfMonth = Integer.decode(dayOfMonth).intValue();
    } catch (Throwable e) {
      logger.error("Invalid value for dayOfMonth parameter: \"" + dayOfMonth + "\"", e);
    }
  }

  public String getWeekDayN()
  {
    return weekDayN;
  }

  public void setWeekDayN(String weekDayN)
  {
    this.weekDayN = weekDayN;
  }

  public String getWeekDay()
  {
    return weekDay;
  }

  public void setWeekDay(String weekDay)
  {
    this.weekDay = weekDay;
  }

  public String getIntervalTime()
  {
    return intervalTime;
  }

  public void setIntervalTime(String intervalTime)
  {
    this.intervalTime = intervalTime;
  }

  public boolean isMonthChecked(String month)
  {
    return checkedMonthsSet.contains(month);
  }

  public String[] getCheckedMonths()
  {
    return checkedMonths;
  }

  public void setCheckedMonths(String[] checkedMonths)
  {
    this.checkedMonths = checkedMonths;
  }

  public String getMonthlyType()
  {
    return monthlyType;
  }

  public void setMonthlyType(String monthlyType)
  {
    this.monthlyType = monthlyType;
  }

  public String[] getCheckedWeekDays()
  {
    return checkedWeekDays;
  }

  public void setCheckedWeekDays(String[] checkedWeekDays)
  {
    this.checkedWeekDays = checkedWeekDays;
  }

  public boolean isWeekDayChecked(String weekday)
  {
    return checkedWeekDaysSet.contains(weekday);
  }

  public String[] getCheckedTasks()
  {
    return checkedTasks;
  }

  public void setCheckedTasks(String[] checkedTasks)
  {
    this.checkedTasks = checkedTasks;
  }

  public boolean isTaskChecked(String task)
  {
    return checkedTasksSet.contains(task);
  }

  public Collection getAllTasks()
  {
    return new SortedList(getConfig().getSectionChildShortSectionNames(TaskDataSource.TASKS_PREFIX));
  }
}

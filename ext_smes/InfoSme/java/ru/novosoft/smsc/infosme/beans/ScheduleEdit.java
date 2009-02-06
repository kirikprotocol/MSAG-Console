package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.infosme.backend.config.schedules.*;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataItem;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataSource;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskQuery;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;

import javax.servlet.http.HttpServletRequest;
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

  private String name = "";
  private String execute = "";
  private String startDateTime = "";
  private String endDateTime = "";
  private int everyNDays = 0;
  private int everyNWeeks = 0;
  private int dayOfMonth = 0;
  private String weekDayN = "";
  private String weekDay = "";
  private String intervalTime = "";

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
          return error("infosme.error.no_schedule");

        try {
          Schedule schedule = getInfoSmeConfig().getSchedule(name);
          execute = Schedule.getExecuteStr(schedule.getExecute());
          checkedTasksSet = schedule.getTasks();
          startDateTime = schedule.getStartDateTimeStr();
          switch (schedule.getExecute()) {
            case Schedule.EXECUTE_ONCE:
              break;
            case Schedule.EXECUTE_DAILY:
              endDateTime = ((ScheduleEndDateTime) schedule).getEndDateTimeStr();
              everyNDays = ((ScheduleDaily) schedule).getEveryNDays();
              break;
            case Schedule.EXECUTE_WEEKLY:
              endDateTime = ((ScheduleEndDateTime) schedule).getEndDateTimeStr();
              everyNWeeks = ((ScheduleWeekly) schedule).getEveryNWeeks();
              checkedWeekDaysSet = new HashSet(((ScheduleWeekly) schedule).getWeekDays());
              break;
            case Schedule.EXECUTE_MONTHLY_DAY:
              endDateTime = ((ScheduleEndDateTime) schedule).getEndDateTimeStr();
              checkedMonthsSet = ((ScheduleMonthly) schedule).getMonthes();
              monthlyType = "day";
              dayOfMonth = ((ScheduleMonthlyDay) schedule).getDayOfMonth();
              break;
            case Schedule.EXECUTE_MONTHLY_WEEK:
              endDateTime = ((ScheduleEndDateTime) schedule).getEndDateTimeStr();
              checkedMonthsSet = ((ScheduleMonthly) schedule).getMonthes();
              monthlyType = "week";
              weekDayN = ((ScheduleMonthlyWeek) schedule).getWeekDayN();
              weekDay = ((ScheduleMonthlyWeek) schedule).getWeekDay();
              break;
            case Schedule.EXECUTE_INTERVAL:
              endDateTime = ((ScheduleEndDateTime) schedule).getEndDateTimeStr();
              intervalTime = ((ScheduleInterval) schedule).getIntervalTime();
              break;
            default:
              return error("infosme.error.no_schedule_type");
          }

          oldSchedule = name;
        } catch (Exception e) {
          logger.error("Could not init bean", e);
          return error("infosme.error.init", e);
        }
      }
    }

    checkedMonthsSet.addAll(Arrays.asList(checkedMonths));
    checkedWeekDaysSet.addAll(Arrays.asList(checkedWeekDays));
    checkedTasksSet.addAll(Arrays.asList(checkedTasks));

    return result;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
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
      return error("infosme.error.no_schedule_name");

    try {
      Schedule schedule = Schedule.getInstance(name, execute, checkedTasksSet, startDateTime, endDateTime, everyNDays,
                                               everyNWeeks, checkedWeekDaysSet, "day".equalsIgnoreCase(monthlyType), checkedMonthsSet,
                                               dayOfMonth, weekDayN, weekDay, intervalTime);

      if (create) {
        if (getInfoSmeConfig().containsSchedule(schedule.getId()))
          return error("infosme.error.exist_schedule", name);
      } else {
        if (!oldSchedule.equals(name)) {
          if (getInfoSmeConfig().containsSchedule(schedule.getId()))
            return error("infosme.error.exist_schedule", name);
        }
        getInfoSmeConfig().removeSchedule(oldSchedule);
      }

      getInfoSmeConfig().addSchedule(schedule);
    } catch (Throwable e) {
      logger.error("Could not store schedule", e);
      error("infosme.error.store_schedule", e);
    }
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
    TaskDataSource ds = new TaskDataSource(getInfoSme(), getInfoSmeConfig());
    QueryResultSet rs = ds.query(new TaskQuery(new Filter() {
      public boolean isEmpty() {
        return false;
      }
      public boolean isItemAllowed(DataItem item) {
        return !((TaskDataItem)item).isDelivery();
      }
    }, 100, "name", 0));
    
    List result = new ArrayList(rs.size());
    for (int i=0; i<rs.size(); i++)
      result.add(((TaskDataItem)rs.get(i)).getId());

    return result;
  }

  public String getTaskName(String taskId)
  {
    return getInfoSmeConfig().getTask(taskId).getName();
  }
}

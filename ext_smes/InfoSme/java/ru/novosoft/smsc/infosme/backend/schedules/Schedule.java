package ru.novosoft.smsc.infosme.backend.schedules;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.tables.schedules.ScheduleDataSource;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.config.Config;

import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 06.10.2003
 * Time: 15:23:08
 */
public abstract class Schedule
{
  public static final byte EXECUTE_UNKNOWN = -1;
  public static final byte EXECUTE_ONCE = 0;
  public static final byte EXECUTE_DAILY = 1;
  public static final byte EXECUTE_WEEKLY = 2;
  public static final byte EXECUTE_MONTHLY_DAY = 3;
  public static final byte EXECUTE_MONTHLY_WEEK = 4;
  public static final byte EXECUTE_INTERVAL = 5;

  public static final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";

  private String id = "";
  private byte execute = EXECUTE_UNKNOWN;
  private Collection tasks = new ArrayList();
  private Date startDateTime = new Date();

  protected final String prefix;
  protected static final DateFormat dateFormat = new SimpleDateFormat(DATE_FORMAT);

  private Schedule(String id)
  {
    this.id = id;
    prefix = createPrefix(id);
  }

  public Schedule(String id, byte execute, Collection tasks, Date startDateTime)
  {
    this(id);
    this.execute = execute;
    this.tasks = tasks;
    this.startDateTime = startDateTime;
  }

  public Schedule(String id, byte execute, Config config) throws Config.ParamNotFoundException, Config.WrongParamTypeException, ParseException
  {
    this(id);
    this.execute = execute;
    Functions.addValuesToCollection(this.tasks, config.getString(prefix + ".tasks"), ",", true);
    this.startDateTime = dateFormat.parse(config.getString(prefix + ".startDateTime"));
  }

  public static Schedule getInstance(String id, Config config) throws Config.ParamNotFoundException, Config.WrongParamTypeException, ParseException, AdminException
  {
    final String prefix = createPrefix(id);
    String executeStr = config.getString(prefix + ".execute");
    if ("once".equalsIgnoreCase(executeStr)) {
      return new ScheduleOnce(id, config);
    } else if ("daily".equalsIgnoreCase(executeStr)) {
      return new ScheduleDaily(id, config);
    } else if ("weekly".equalsIgnoreCase(executeStr)) {
      return new ScheduleWeekly(id, config);
    } else if ("monthly".equalsIgnoreCase(executeStr)) {
      if (config.containsParameter(prefix + ".dayOfMonth")) {
        return new ScheduleMonthlyDay(id, config);
      } else {
        return new ScheduleMonthlyWeek(id, config);
      }
    } else if ("interval".equalsIgnoreCase(executeStr)) {
      return new ScheduleInterval(id, config);
    } else {
      throw new AdminException("Unknown schedule type \"" + executeStr + '"');
    }
  }

  public static Schedule getInstance(String id, String executeStr,
                                     Collection tasks, String startDateTime,
                                     String endDateTime,
                                     int everyNDays,
                                     int everyNWeeks, Collection weekDays,
                                     boolean montlyTypeIsDay, Collection monthes,
                                     int dayOfMonth,
                                     String weekDayN, String weekDay,
                                     String intervalTime
                                     )
          throws AdminException, ParseException
  {
    if ("once".equalsIgnoreCase(executeStr)) {
      return new ScheduleOnce(id, tasks, dateFormat.parse(startDateTime));
    } else if ("daily".equalsIgnoreCase(executeStr)) {
      return new ScheduleDaily(id, tasks, dateFormat.parse(startDateTime), endDateTime, everyNDays);
    } else if ("weekly".equalsIgnoreCase(executeStr)) {
      return new ScheduleWeekly(id, tasks, dateFormat.parse(startDateTime), endDateTime, everyNWeeks, weekDays);
    } else if ("monthly".equalsIgnoreCase(executeStr)) {
      if (montlyTypeIsDay) {
        return new ScheduleMonthlyDay(id, tasks, dateFormat.parse(startDateTime), endDateTime, monthes, dayOfMonth);
      } else {
        return new ScheduleMonthlyWeek(id, tasks, dateFormat.parse(startDateTime), endDateTime, monthes, weekDayN, weekDay);
      }
    } else if ("interval".equalsIgnoreCase(executeStr)) {
      return new ScheduleInterval(id, tasks, dateFormat.parse(startDateTime), endDateTime, intervalTime);
    } else {
      throw new AdminException("Unknown schedule type \"" + executeStr + '"');
    }
  }

  protected static String createPrefix(String schedId)
  {
    return ScheduleDataSource.SCHEDULES_PREFIX + '.' + StringEncoderDecoder.encodeDot(schedId);
  }

  /**
   * stores properties to config<br>
   * <B>MUST BE OVERLOADED IN CHILDS</B>
   * @param config
   */
  public void storeToConfig(Config config)
  {
    config.setString(prefix + ".execute", getExecuteStr(execute));
    config.setString(prefix + ".tasks", Functions.collectionToString(tasks, ","));
    config.setString(prefix + ".startDateTime", dateFormat.format(startDateTime));
  }

  public boolean isContainsInConfig(Config config)
  {
    return config.containsSection(prefix);
  }

  public static void removeScheduleFromConfig(String scheduleId, Config config)
  {
    config.removeSection(createPrefix(scheduleId));
  }

  public void removeFromConfig(Config config)
  {
    removeScheduleFromConfig(this.id, config);
  }

  public static String getExecuteStr(byte execute)
  {
    switch (execute) {
      case EXECUTE_ONCE:
        return "once";
      case EXECUTE_DAILY:
        return "daily";
      case EXECUTE_WEEKLY:
        return "weekly";
      case EXECUTE_MONTHLY_DAY:
      case EXECUTE_MONTHLY_WEEK:
        return "monthly";
      case EXECUTE_INTERVAL:
        return "interval";
      default:
        return "unknown";
    }
  }

  public boolean equals(Object obj)
  {
    if (obj instanceof Schedule) {
      Schedule schedule = (Schedule) obj;
      return this.id.equals(schedule.id) && this.execute == schedule.execute && this.tasks.equals(schedule.tasks) && this.startDateTime.equals(schedule.startDateTime);
    } else
      return false;
  }

  /************************************** properties ******************************/

  public String getId()
  {
    return id;
  }

  public void setId(String id)
  {
    this.id = id;
  }

  public byte getExecute()
  {
    return execute;
  }

  public void setExecute(byte execute)
  {
    this.execute = execute;
  }

  public Collection getTasks()
  {
    return tasks;
  }

  public void setTasks(Collection tasks)
  {
    this.tasks = tasks;
  }

  public Date getStartDateTime()
  {
    return startDateTime;
  }

  public String getStartDateTimeStr()
  {
    return dateFormat.format(startDateTime);
  }

  public void setStartDateTime(Date startDateTime)
  {
    this.startDateTime = startDateTime;
  }
}

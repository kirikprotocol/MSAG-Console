package ru.novosoft.smsc.infosme.backend.schedules;

import ru.novosoft.smsc.util.config.Config;

import java.text.ParseException;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 06.10.2003
 * Time: 15:43:34
 */
public class ScheduleMonthlyWeek extends ScheduleMonthly
{
  private String weekDayN = "";
  private String weekDay = "";

  public ScheduleMonthlyWeek(String id, Collection tasks, Date startDateTime, Date endDateTime, Collection monthes, String weekDayN, String weekDay)
  {
    super(id, EXECUTE_MONTHLY_WEEK, tasks, startDateTime, endDateTime, monthes);
    this.weekDayN = weekDayN;
    this.weekDay = weekDay;
  }

  public ScheduleMonthlyWeek(String id, Collection tasks, Date startDateTime, String endDateTime, Collection monthes, String weekDayN, String weekDay) throws ParseException
  {
    super(id, EXECUTE_MONTHLY_WEEK, tasks, startDateTime, endDateTime, monthes);
    this.weekDayN = weekDayN;
    this.weekDay = weekDay;
  }

  public ScheduleMonthlyWeek(String id, Config config) throws Config.ParamNotFoundException, ParseException, Config.WrongParamTypeException
  {
    super(id, EXECUTE_MONTHLY_WEEK, config);
    this.weekDayN = config.getString(prefix + ".weekDayN");
    this.weekDay = config.getString(prefix + ".weekDay");
  }

  public void storeToConfig(Config config)
  {
    super.storeToConfig(config);
    config.setString(prefix + ".weekDayN", weekDayN);
    config.setString(prefix + ".weekDay", weekDay);
  }

  public boolean equals(Object obj)
  {
    if (obj instanceof ScheduleMonthlyWeek) {
      ScheduleMonthlyWeek schedule = (ScheduleMonthlyWeek) obj;
      return super.equals(schedule) && this.weekDay.equals(schedule.weekDay) && this.weekDayN.equals(schedule.weekDayN);
    } else
      return false;
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
}

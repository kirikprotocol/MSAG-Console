package ru.novosoft.smsc.infosme.backend.schedules;

import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.config.Config;

import java.text.ParseException;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 06.10.2003
 * Time: 15:35:04
 */
public class ScheduleWeekly extends ScheduleEndDateTime
{
  private int everyNWeeks = 0;
  private Collection weekDays = new ArrayList();

  public ScheduleWeekly(String id, Collection tasks, Date startDateTime, Date endDateTime, int everyNWeeks, Collection weekDays)
  {
    super(id, EXECUTE_WEEKLY, tasks, startDateTime, endDateTime);
    this.everyNWeeks = everyNWeeks;
    this.weekDays = weekDays;
  }

  public ScheduleWeekly(String id, Collection tasks, Date startDateTime, String endDateTime, int everyNWeeks, Collection weekDays) throws ParseException
  {
    super(id, EXECUTE_WEEKLY, tasks, startDateTime, endDateTime);
    this.everyNWeeks = everyNWeeks;
    this.weekDays = weekDays;
  }

  public ScheduleWeekly(String id, Config config) throws Config.ParamNotFoundException, ParseException, Config.WrongParamTypeException
  {
    super(id, EXECUTE_WEEKLY, config);
    this.everyNWeeks = config.getInt(prefix + ".everyNWeeks");
    Functions.addValuesToCollection(this.weekDays, config.getString(prefix + ".weekDays"), ",", true);
  }

  public void storeToConfig(Config config)
  {
    super.storeToConfig(config);
    config.setInt(prefix + ".everyNWeeks", everyNWeeks);
    config.setString(prefix + ".weekDays", Functions.collectionToString(weekDays, ","));
  }

  public boolean equals(Object obj)
  {
    if (obj instanceof ScheduleWeekly) {
      ScheduleWeekly schedule = (ScheduleWeekly) obj;
      return super.equals(schedule) && this.everyNWeeks == schedule.everyNWeeks && this.weekDays.equals(schedule.weekDays);
    } else
      return false;
  }

  public int getEveryNWeeks()
  {
    return everyNWeeks;
  }

  public void setEveryNWeeks(int everyNWeeks)
  {
    this.everyNWeeks = everyNWeeks;
  }

  public Collection getWeekDays()
  {
    return weekDays;
  }

  public void setWeekDays(Collection weekDays)
  {
    this.weekDays = weekDays;
  }
}

package ru.novosoft.smsc.infosme.backend.schedules;

import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.config.Config;

import java.text.ParseException;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 06.10.2003
 * Time: 15:35:32
 */
public abstract class ScheduleMonthly extends ScheduleEndDateTime
{
  private Collection monthes = new ArrayList();

  public ScheduleMonthly(String id, byte execute, Collection tasks, Date startDateTime, Date endDateTime, Collection monthes)
  {
    super(id, execute, tasks, startDateTime, endDateTime);
    this.monthes = monthes;
  }

  public ScheduleMonthly(String id, byte execute, Collection tasks, Date startDateTime, String endDateTime, Collection monthes) throws ParseException
  {
    super(id, execute, tasks, startDateTime, endDateTime);
    this.monthes = monthes;
  }

  public ScheduleMonthly(String id, byte execute, Config config) throws Config.ParamNotFoundException, ParseException, Config.WrongParamTypeException
  {
    super(id, execute, config);
    Functions.addValuesToCollection(this.monthes, config.getString(prefix + ".monthes"), ",", true);
  }

  /**
   * stores properties to config<br>
   * <B>MUST BE OVERLOADED IN CHILDS</B>
   *
   * @param config
   */
  public void storeToConfig(Config config)
  {
    super.storeToConfig(config);
    config.setString(prefix + ".monthes", Functions.collectionToString(monthes, ","));
  }

  public boolean equals(Object obj)
  {
    if (obj instanceof ScheduleMonthly) {
      ScheduleMonthly schedule = (ScheduleMonthly) obj;
      return super.equals(schedule) && this.monthes.equals(schedule.monthes);
    } else
      return false;
  }

  public Collection getMonthes()
  {
    return monthes;
  }

  public void setMonthes(Collection monthes)
  {
    this.monthes = monthes;
  }
}

package ru.novosoft.smsc.infosme.backend.schedules;

import ru.novosoft.smsc.util.config.Config;

import java.text.ParseException;
import java.util.Collection;
import java.util.Date;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 06.10.2003
 * Time: 15:34:45
 */
public class ScheduleDaily extends ScheduleEndDateTime
{
  private int everyNDays = 0;

  public ScheduleDaily(String id, Collection tasks, Date startDateTime, Date endDateTime, int everyNDays)
  {
    super(id, EXECUTE_DAILY, tasks, startDateTime, endDateTime);
    this.everyNDays = everyNDays;
  }

  public ScheduleDaily(String id, Collection tasks, Date startDateTime, String endDateTime, int everyNDays) throws ParseException
  {
    super(id, EXECUTE_DAILY, tasks, startDateTime, endDateTime);
    this.everyNDays = everyNDays;
  }

  public ScheduleDaily(String id, Config config) throws Config.ParamNotFoundException, ParseException, Config.WrongParamTypeException
  {
    super(id, EXECUTE_DAILY, config);
    this.everyNDays = config.getInt(prefix + ".everyNDays");
  }

  public void storeToConfig(Config config)
  {
    super.storeToConfig(config);
    config.setInt(prefix + ".everyNDays", everyNDays);
  }

  public boolean equals(Object obj)
  {
    if (obj instanceof ScheduleDaily) {
      ScheduleDaily daily = (ScheduleDaily) obj;
      return super.equals(obj) && this.everyNDays == daily.everyNDays;
    } else
      return false;
  }

  /**
   * *********************************** properties *****************************
   */

  public int getEveryNDays()
  {
    return everyNDays;
  }

  public void setEveryNDays(int everyNDays)
  {
    this.everyNDays = everyNDays;
  }
}

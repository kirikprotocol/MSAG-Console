package ru.novosoft.smsc.infosme.backend.schedules;

import ru.novosoft.smsc.util.config.Config;

import java.text.ParseException;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 06.10.2003
 * Time: 15:35:48
 */
public class ScheduleInterval extends ScheduleEndDateTime
{
  private String intervalTime = "";

  public ScheduleInterval(String id, Collection tasks, Date startDateTime, Date endDateTime, String intervalTime)
  {
    super(id, EXECUTE_INTERVAL, tasks, startDateTime, endDateTime);
    this.intervalTime = intervalTime;
  }

  public ScheduleInterval(String id, Collection tasks, Date startDateTime, String endDateTime, String intervalTime) throws ParseException
  {
    super(id, EXECUTE_INTERVAL, tasks, startDateTime, endDateTime);
    this.intervalTime = intervalTime;
  }

  public ScheduleInterval(String id, Config config) throws Config.ParamNotFoundException, ParseException, Config.WrongParamTypeException
  {
    super(id, EXECUTE_INTERVAL, config);
    this.intervalTime = config.getString(prefix + ".intervalTime");
  }

  public void storeToConfig(Config config)
  {
    super.storeToConfig(config);
    config.setString(prefix + ".intervalTime", intervalTime);
  }

  public boolean equals(Object obj)
  {
    if (obj instanceof ScheduleInterval) {
      ScheduleInterval schedule = (ScheduleInterval) obj;
      return super.equals(schedule) && this.intervalTime.equals(schedule.intervalTime);
    } else
      return false;
  }

  public String getIntervalTime()
  {
    return intervalTime;
  }

  public void setIntervalTime(String intervalTime)
  {
    this.intervalTime = intervalTime;
  }
}

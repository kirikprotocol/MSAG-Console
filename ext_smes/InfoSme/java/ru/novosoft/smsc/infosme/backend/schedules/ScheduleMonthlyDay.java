package ru.novosoft.smsc.infosme.backend.schedules;

import ru.novosoft.smsc.util.config.Config;

import java.text.ParseException;
import java.util.Collection;
import java.util.Date;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 06.10.2003
 * Time: 15:43:10
 */
public class ScheduleMonthlyDay extends ScheduleMonthly
{
  private int dayOfMonth = 0;

  public ScheduleMonthlyDay(String id, Collection tasks, Date startDateTime, Date endDateTime, Collection monthes, int dayOfMonth)
  {
    super(id, EXECUTE_MONTHLY_DAY, tasks, startDateTime, endDateTime, monthes);
    this.dayOfMonth = dayOfMonth;
  }

  public ScheduleMonthlyDay(String id, Collection tasks, Date startDateTime, String endDateTime, Collection monthes, int dayOfMonth) throws ParseException
  {
    super(id, EXECUTE_MONTHLY_DAY, tasks, startDateTime, endDateTime, monthes);
    this.dayOfMonth = dayOfMonth;
  }

  public ScheduleMonthlyDay(String id, Config config) throws Config.ParamNotFoundException, ParseException, Config.WrongParamTypeException
  {
    super(id, EXECUTE_MONTHLY_DAY, config);
    this.dayOfMonth = config.getInt(prefix + ".dayOfMonth");
  }

  public void storeToConfig(Config config)
  {
    super.storeToConfig(config);
    config.setInt(prefix + ".dayOfMonth", dayOfMonth);
  }

  public boolean equals(Object obj)
  {
    if (obj instanceof ScheduleMonthlyDay) {
      ScheduleMonthlyDay schedule = (ScheduleMonthlyDay) obj;
      return super.equals(schedule) && this.dayOfMonth == schedule.dayOfMonth;
    } else
      return false;
  }

  public int getDayOfMonth()
  {
    return dayOfMonth;
  }

  public void setDayOfMonth(int dayOfMonth)
  {
    this.dayOfMonth = dayOfMonth;
  }
}

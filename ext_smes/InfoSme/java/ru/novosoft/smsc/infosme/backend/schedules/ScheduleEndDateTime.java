package ru.novosoft.smsc.infosme.backend.schedules;

import ru.novosoft.smsc.util.config.Config;

import java.text.ParseException;
import java.util.Collection;
import java.util.Date;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 06.10.2003
 * Time: 15:38:39
 */
public abstract class ScheduleEndDateTime extends Schedule
{
  private Date endDateTime = null;

  public ScheduleEndDateTime(String id, byte execute, Collection tasks, Date startDateTime, Date endDateTime)
  {
    super(id, execute, tasks, startDateTime);
    this.endDateTime = endDateTime;
  }

  public ScheduleEndDateTime(String id, byte execute, Collection tasks, Date startDateTime, String endDateTime) throws ParseException
  {
    super(id, execute, tasks, startDateTime);
    setEndDateTime(endDateTime);
  }

  public ScheduleEndDateTime(String id, byte execute, Config config) throws Config.ParamNotFoundException, ParseException, Config.WrongParamTypeException
  {
    super(id, execute, config);
    final String parameterName = prefix + ".endDateTime";
    if (config.containsParameter(parameterName))
      setEndDateTime(config.getString(parameterName));
    else
      this.endDateTime = null;
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
    if (endDateTime != null && endDateTime.getTime() != 0)
      config.setString(prefix + ".endDateTime", dateFormat.format(endDateTime));
  }

  public boolean equals(Object obj)
  {
    if (obj instanceof ScheduleEndDateTime) {
      ScheduleEndDateTime schedule = (ScheduleEndDateTime) obj;
      return super.equals(schedule)
             && ((this.endDateTime == null && schedule.endDateTime == null)
                 || (this.endDateTime != null && schedule.endDateTime != null && this.endDateTime.equals(schedule.endDateTime)));
    } else
      return false;
  }

  public Date getEndDateTime()
  {
    return endDateTime;
  }

  public String getEndDateTimeStr()
  {
    return endDateTime == null || endDateTime.getTime() == 0 ? "" : dateFormat.format(endDateTime);
  }

  public void setEndDateTime(Date endDateTime)
  {
    this.endDateTime = endDateTime;
  }

  public void setEndDateTime(String endDateTime) throws ParseException
  {
    this.endDateTime = endDateTime == null || endDateTime.trim().length() == 0 ? null : dateFormat.parse(endDateTime);
  }
}

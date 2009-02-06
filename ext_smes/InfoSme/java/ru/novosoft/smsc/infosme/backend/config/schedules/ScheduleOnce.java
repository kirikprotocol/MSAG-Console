package ru.novosoft.smsc.infosme.backend.config.schedules;

import ru.novosoft.smsc.util.config.Config;

import java.text.ParseException;
import java.util.Collection;
import java.util.Date;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 06.10.2003
 * Time: 15:34:16
 */
public class ScheduleOnce extends Schedule
{
  public ScheduleOnce(String id, Collection tasks, Date startDateTime)
  {
    super(id, EXECUTE_ONCE, tasks, startDateTime);
  }

  ScheduleOnce(String id, Config config) throws Config.ParamNotFoundException, ParseException, Config.WrongParamTypeException
  {
    super(id, EXECUTE_ONCE, config);
  }

  void storeToConfig(Config config)
  {
    super.storeToConfig(config);
  }

  public boolean equals(Object obj)
  {
    if (obj instanceof ScheduleOnce) {
      ScheduleOnce schedule = (ScheduleOnce) obj;
      return super.equals(schedule);
    } else
      return false;
  }
}

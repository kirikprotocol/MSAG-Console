package ru.novosoft.smsc.infosme.backend.tables.schedules;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

/**
 * Created by igork
 * Date: Sep 2, 2003
 * Time: 12:58:13 PM
 */
public class ScheduleDataItem extends AbstractDataItem {
  public ScheduleDataItem(String name, String execute, String startDateTime)
  {
    values.put("name", name);
    values.put("execute", execute);
    values.put("startDateTime", startDateTime);
  }

  public String getName()
  {
    return (String) values.get("name");
  }

  public String getExecute()
  {
    return (String) values.get("execute");
  }

  public String getStartDateTime()
  {
    return (String) values.get("startDateTime");
  }
}

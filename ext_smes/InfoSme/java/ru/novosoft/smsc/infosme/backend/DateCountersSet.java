package ru.novosoft.smsc.infosme.backend;

import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 30.09.2003
 * Time: 13:25:09
 * To change this template use Options | File Templates.
 */

public class DateCountersSet extends CountersSet
{
  private Date date;

  private Vector byHours = new Vector(); // contains HourCountersSet

  DateCountersSet(Date date)
  {
    this.date = date;
  }

  public void addHourStat(HourCountersSet set)
  {
    byHours.addElement(set);
    super.increment(set);
  }

  public Collection getHourStat()
  {
    return byHours;
  }

  public Date getDate()
  {
    return date;
  }
}


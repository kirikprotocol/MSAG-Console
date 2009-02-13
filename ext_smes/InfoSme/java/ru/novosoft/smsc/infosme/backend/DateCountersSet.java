package ru.novosoft.smsc.infosme.backend;

import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 30.09.2003
 * Time: 13:25:09
 * To change this template use Options | File Templates.
 */

public class DateCountersSet extends CountersSet implements Comparable
{
  private Date date;

  private TreeSet byHours = new TreeSet(); // contains HourCountersSet

  public DateCountersSet(Date date)
  {
    this.date = date;
  }

  public void addHourStat(HourCountersSet set)
  {
    byHours.add(set);
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

  public int compareTo(Object o) {
    return date.compareTo(((DateCountersSet)o).date);
  }
}


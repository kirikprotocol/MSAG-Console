package ru.novosoft.smsc.mcisme.backend;

import java.util.Date;
import java.util.Vector;
import java.util.Collection;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 02.04.2004
 * Time: 18:02:24
 * To change this template use Options | File Templates.
 */
public class DateCountersSet extends CountersSet
{
  private Date date;

  private Vector byHours = new Vector(); // contains HourCountersSet

  DateCountersSet(Date date) {
    this.date = date;
  }

  public void addHourStat(HourCountersSet set) {
    byHours.addElement(set);
    super.increment(set);
  }

  public Collection getHourStat() {
    return byHours;
  }

  public Date getDate() {
    return date;
  }

}

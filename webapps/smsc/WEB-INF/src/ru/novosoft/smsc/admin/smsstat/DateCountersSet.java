package ru.novosoft.smsc.admin.smsstat;

/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 10, 2002
 * Time: 4:37:26 PM
 * To change this template use Options | File Templates.
 */

import java.util.Collection;
import java.util.Date;
import java.util.ArrayList;

public class DateCountersSet extends CountersSet
{
  private Date date;
  private ArrayList byHours = new ArrayList(); // contains HourCountersSet

  DateCountersSet(Date date) {
    this.date = date;
  }
  public void addHourStat(HourCountersSet set) {
    byHours.add(set);
    super.increment(set);
  }

  public Collection getHourStat() {
    return byHours;
  }
  public Date getDate() {
    return date;
  }
}

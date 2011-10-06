package ru.novosoft.smsc.admin.stat;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Date;

/**
 * author: Aleksandr Khalitov
 */
public class DateCountersSet extends CountersSet {
  private Date date;
  private ArrayList<HourCountersSet> byHours = new ArrayList<HourCountersSet>(); // contains HourCountersSet

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
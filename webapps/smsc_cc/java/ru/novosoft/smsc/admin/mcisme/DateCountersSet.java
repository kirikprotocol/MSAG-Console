package ru.novosoft.smsc.admin.mcisme;

import java.util.Collection;
import java.util.Date;
import java.util.LinkedList;

/**
 * author: Aleksandr Khalitov
 */
public class DateCountersSet extends CountersSet {

  private final Date date;

  private final Collection<HourCountersSet> byHours = new LinkedList<HourCountersSet>();

  DateCountersSet(Date date) {
    this.date = date;
  }

  public void addHourStat(HourCountersSet set) {
    byHours.add(set);
    super.increment(set);
  }

  public Collection<HourCountersSet> getHourStat() {
    return byHours;
  }

  public Date getDate() {
    return date;
  }
}

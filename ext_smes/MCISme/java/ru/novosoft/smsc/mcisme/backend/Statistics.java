package ru.novosoft.smsc.mcisme.backend;

import java.util.Vector;
import java.util.Collection;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 02.04.2004
 * Time: 17:58:22
 * To change this template use Options | File Templates.
 */
public class Statistics
{
  private CountersSet counters = new CountersSet();
  private Vector countersByDates = new Vector(); // contains DateCountersSet

  public CountersSet getCounters() {
    return counters;
  }

  public void addDateStat(DateCountersSet set) {
    countersByDates.addElement(set);
    counters.increment(set);
  }

  public Collection getCountersByDates() {
    return countersByDates;
  }

}

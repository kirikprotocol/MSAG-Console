package ru.novosoft.smsc.infosme.backend;

import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 30.09.2003
 * Time: 13:21:38
 * To change this template use Options | File Templates.
 */

public class Statistics
{
  private CountersSet counters = new CountersSet();
  private Vector countersByDates = new Vector(); // contains DateCountersSet

  public CountersSet getCounters()
  {
    return counters;
  }

  public void addDateStat(DateCountersSet set)
  {
    countersByDates.addElement(set);
    counters.increment(set);
  }

  public Collection getCountersByDates()
  {
    return countersByDates;
  }
}

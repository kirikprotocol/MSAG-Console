package ru.novosoft.smsc.infosme.backend;

import java.util.Collection;
import java.util.Vector;
import java.util.TreeSet;

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
  private TreeSet countersByDates = new TreeSet(); // contains DateCountersSet

  public CountersSet getCounters()
  {
    return counters;
  }

  public void addDateStat(DateCountersSet set)
  {
    countersByDates.add(set);
    counters.increment(set);
  }

  public Collection getCountersByDates()
  {
    return countersByDates;
  }
}

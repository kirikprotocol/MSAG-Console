package ru.novosoft.smsc.admin.mcisme;

import java.util.Collection;
import java.util.LinkedList;

/**
 * author: Aleksandr Khalitov
 */
public class MessageStatistics {

  private final CountersSet counters = new CountersSet();
  private final Collection<DateCountersSet> countersByDates = new LinkedList<DateCountersSet>();

  public CountersSet getCounters() {
    return counters;
  }

  public void addDateStat(DateCountersSet set) {
    countersByDates.add(set);
    counters.increment(set);
  }

  public Collection<DateCountersSet> getCountersByDates() {
    return countersByDates;
  }
}

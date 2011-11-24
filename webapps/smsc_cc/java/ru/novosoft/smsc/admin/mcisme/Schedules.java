package ru.novosoft.smsc.admin.mcisme;

import java.util.ArrayList;
import java.util.Collection;
import java.util.LinkedList;

/**
 * author: Aleksandr Khalitov
 */
public class Schedules {



  private final Collection<Counter> counters = new LinkedList<Counter>();

  public Collection<Counter> getCounters() {
    return new ArrayList<Counter>(counters);
  }

  public void addCounter(Counter c) {
    if(c != null) {
      this.counters.add(c);
    }
  }

  static class Counter {
    private final long time;
    private final int count;

    Counter(long time, int count) {
      this.time = time;
      this.count = count;
    }

    public long getTime() {
      return time;
    }

    public int getCount() {
      return count;
    }
  }
}

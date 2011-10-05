package ru.novosoft.smsc.admin.stat;

import java.util.Collection;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

/**
 * author: Aleksandr Khalitov
 */
public class Statistics extends ExtendedCountersSet {
  private List<DateCountersSet> byDates = new LinkedList<DateCountersSet>(); // contains DateCountersSet
  private List<SmeIdCountersSet> bySmeId = new LinkedList<SmeIdCountersSet>(); // contains SmeIdCountersSet
  private List<RouteIdCountersSet> byRouteId = new LinkedList<RouteIdCountersSet>(); // contains RouteIdCountersSet

  Statistics() {
  }

  Statistics(long accepted, long rejected, long delivered, long failed, long rescheduled, long temporal, long i, long o) {
    super(accepted, rejected, delivered, failed, rescheduled, temporal, i, o);
  }

  public CountersSet getTotal() {
    return this;
  }
  void addDateStat(DateCountersSet set) {
    byDates.add(set);
    this.increment(set); // increments total statistics
  }

  public Collection<DateCountersSet> getDateStat() {
    return byDates;
  }
  public Collection<ErrorCounterSet> getErrorStat() {
    return this.getErrors();
  }


  void addSmeIdCollection(Collection<SmeIdCountersSet> col){
    if(col != null) {
      bySmeId.addAll(col);
    }
  }
  public Collection<SmeIdCountersSet> getSmeIdStat() {
    Collections.sort(bySmeId);
    return bySmeId;
  }

  void addRouteIdCollection(Collection<RouteIdCountersSet> col) {
    if(col != null) {
      byRouteId.addAll(col);
    }
  }

  public Collection<RouteIdCountersSet> getRouteIdStat() {
    Collections.sort(byRouteId);
    return byRouteId;
  }
}

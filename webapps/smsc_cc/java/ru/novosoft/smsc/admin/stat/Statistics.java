package ru.novosoft.smsc.admin.stat;

import java.util.Collection;
import java.util.LinkedList;
import java.util.List;

/**
 * author: Aleksandr Khalitov
 */
public class Statistics extends ExtendedCountersSet {

  private List<DateCountersSet> byDates = new LinkedList<DateCountersSet>();
  private List<SmeIdCountersSet> bySmeId = new LinkedList<SmeIdCountersSet>();
  private List<RouteIdCountersSet> byRouteId = new LinkedList<RouteIdCountersSet>();

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

  public List<DateCountersSet> getGeneralStat() {
    return byDates;
  }

  public int generalStatSize() {
    return byDates.size();
  }

  public int smeStatSize() {
    return bySmeId.size();
  }

  public int routeStatSize() {
    return byRouteId.size();
  }

  void addSmeIdCollection(Collection<SmeIdCountersSet> col){
    if(col != null) {
      bySmeId.addAll(col);
    }
  }
  public List<SmeIdCountersSet> getSmeIdStat() {
    return bySmeId;
  }

  void addRouteIdCollection(Collection<RouteIdCountersSet> col) {
    if(col != null) {
      byRouteId.addAll(col);
    }
  }

  public List<RouteIdCountersSet> getRouteIdStat() {
    return byRouteId;
  }
}

package ru.sibinco.smppgw.backend.stat.stat;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 11.08.2004
 * Time: 14:43:47
 * To change this template use File | Settings | File Templates.
 */
public class Statistics
{
  private ExtendedCountersSet total = new ExtendedCountersSet();
  private ArrayList byDates    = new ArrayList(100); // contains DateCountersSet
  private ArrayList bySmeId    = new ArrayList(100); // contains SmeIdCountersSet
  private ArrayList byRouteId  = new ArrayList(100); // contains RouteIdCountersSet

  private boolean full = false;

  public CountersSet getTotal() {
    return total;
  }

  public void addDateStat(DateCountersSet set) {
    byDates.add(set);
    total.increment(set);
  }
  public Collection getDateStat() {
    return byDates;
  }

  public void addErrorStat(ErrorCounterSet set) {
    total.addError(set);
  }
  public Collection getErrorStat() {
    return total.getErrors();
  }

  public void addSmeIdStat(SmeIdCountersSet set) {
    bySmeId.add(set);
  }
  public void addSmeIdCollection(Collection col) {
    bySmeId.addAll(col);
  }
  public Collection getSmeIdStat() {
    Collections.sort(bySmeId);
    return bySmeId;
  }

  public void addRouteIdStat(RouteIdCountersSet set) {
    byRouteId.add(set);
  }
  public void addRouteIdCollection(Collection col) {
    byRouteId.addAll(col);
  }
  public Collection getRouteIdStat() {
    Collections.sort(byRouteId);
    return byRouteId;
  }

  public boolean isFull() {
    return full;
  }
  public void setFull(boolean full) {
    this.full = full;
  }
}

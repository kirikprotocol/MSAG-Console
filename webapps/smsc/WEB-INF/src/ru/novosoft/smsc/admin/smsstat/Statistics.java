package ru.novosoft.smsc.admin.smsstat;

/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 5, 2002
 * Time: 2:32:02 PM
 * To change this template use Options | File Templates.
 */

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;

public class Statistics extends ExtendedCountersSet
{
  private ArrayList byDates = new ArrayList(100); // contains DateCountersSet
  private ArrayList bySmeId = new ArrayList(100); // contains SmeIdCountersSet
  private ArrayList byRouteId = new ArrayList(100); // contains RouteIdCountersSet

  public CountersSet getTotal() {
    return this;
  }
  public void addDateStat(DateCountersSet set) {
    byDates.add(set);
    this.increment(set); // increments total statistics
  }

  public Collection getDateStat() // index.jsp
  {
    return byDates;
  }
  public Collection getErrorStat() // index.jsp
  {
    return this.getErrors();
  }

 
  public void addSmeIdCollection(Collection col)
  {
    bySmeId.addAll(col);
  }
  public Collection getSmeIdStat() // index.jsp
  {
    Collections.sort(bySmeId);
    return bySmeId;
  }

  public void addRouteIdCollection(Collection col)
  {
    byRouteId.addAll(col);
  }
  public Collection getRouteIdStat() // index.jsp
  {
    Collections.sort(byRouteId);
    return byRouteId;
  }
}

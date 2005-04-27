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

public class Statistics
{
  private ExtendedCountersSet total = new ExtendedCountersSet();
  private ArrayList byDates = new ArrayList(100); // contains DateCountersSet
  private ArrayList bySmeId = new ArrayList(100); // contains SmeIdCountersSet
  private ArrayList byRouteId = new ArrayList(100); // contains RouteIdCountersSet

  public CountersSet getTotal()
  {
    return total;
  }

  public void addDateStat(DateCountersSet set)
  {
    byDates.add(set);
    total.increment(set);
  }
    public void mergeDateStat(DateCountersSet set)
  {
    byDates.add(set);
    total.increment(set);
  }
  public Collection getDateStat()
  {
    return byDates;
  }

  public void addErrorStat(ErrorCounterSet set)
  {
    total.addError(set);
  }
   public void putErrorStat(ErrorCounterSet set)
  {
    total.putErr(set.errcode,set);
  }
    public void addAllErrors(Collection err)
  {
    total.addAllErrors(err);
  }
  public Collection getErrorStat()
  {
    return total.getErrors();
  }
   public ErrorCounterSet getErrStat(int errcode)
  {
    return total.getErr(errcode);
  }
  public void addSmeIdStat(SmeIdCountersSet set)
  {
    bySmeId.add(set);
  }

  public void addSmeIdCollection(Collection col)
  {
    bySmeId.addAll(col);
  }

  public Collection getSmeIdStat()
  {
    Collections.sort(bySmeId);
    return bySmeId;
  }

  public void addRouteIdStat(RouteIdCountersSet set)
  {
    byRouteId.add(set);
  }

  public void addRouteIdCollection(Collection col)
  {
    byRouteId.addAll(col);
  }

  public Collection getRouteIdStat()
  {
    Collections.sort(byRouteId);
    return byRouteId;
  }
}

package ru.sibinco.smppgw.backend.stat.stat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 26.08.2004
 * Time: 14:00:41
 * To change this template use File | Settings | File Templates.
 */
public class RouteIdCountersSet extends ExtendedCountersSet implements Comparable
{
  protected String routeid;

  public RouteIdCountersSet(String routeid)
   {
     this.routeid = routeid;
   }

  public RouteIdCountersSet(long accepted, long rejected, long delivered,
                            long tempError, long permError, String routeid)
  {
    super(accepted, rejected, delivered, tempError, permError);
    this.routeid = routeid;
  }

  public RouteIdCountersSet(long accepted, long rejected, long delivered, long tempError, long permError,
                            long smsTrOk, long smsTrFailed, long smsTrBilled,
                            long ussdTrFromScOk, long ussdTrFromScFailed, long ussdTrFromScBilled,
                            long ussdTrFromSmeOk, long ussdTrFromSmeFailed, long ussdTrFromSmeBilled,
                            String routeid)
  {
    super(accepted, rejected, delivered, tempError, permError,
          smsTrOk, smsTrFailed, smsTrBilled,
          ussdTrFromScOk, ussdTrFromScFailed, ussdTrFromScBilled,
          ussdTrFromSmeOk, ussdTrFromSmeFailed, ussdTrFromSmeBilled);
    this.routeid = routeid;
  }

  public int compareTo(Object o) {
    if (o == null || routeid == null || !(o instanceof RouteIdCountersSet)) return -1;
    return routeid.compareTo(((RouteIdCountersSet)o).routeid);
  }

  public String getRouteid() {
    return routeid;
  }
}

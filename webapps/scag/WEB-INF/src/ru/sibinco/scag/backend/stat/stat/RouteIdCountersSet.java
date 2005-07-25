package ru.sibinco.scag.backend.stat.stat;

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
  public int providerId = -1;

  public RouteIdCountersSet(String routeid)
   {
     this.routeid = routeid;
   }

  public RouteIdCountersSet(long accepted, long rejected, long delivered,
                     long gw_rejected, long failed, long billingOk,
                     long billingFailed, long recieptOk, long recieptFailed, String routeid)
  {
    super(accepted, rejected, delivered, gw_rejected, failed, billingOk, billingFailed, recieptOk, recieptFailed);
    this.routeid = routeid;
  }

  public int compareTo(Object o) {
    if (o == null || routeid == null || !(o instanceof RouteIdCountersSet)) return -1;
    return routeid.compareTo(((RouteIdCountersSet)o).routeid);
  }

  public String getRouteid() {
    return routeid;
  }

    public int getProviderId() {
        return providerId;
    }

    public void setProviderId(int providerId) {
        this.providerId = providerId;
    }
}

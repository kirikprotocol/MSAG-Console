/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 10, 2002
 * Time: 4:38:25 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.smsstat;

public class RouteIdCountersSet extends ExtendedCountersSet
{
  public String routeid;

  public RouteIdCountersSet(String routeid)
  {
    this.routeid = routeid;
  }

  public RouteIdCountersSet(int accepted, int rejected, int delivered,
                            int failed, int rescheduled, int temporal, String routeid)
  {
    super(accepted, rejected, delivered, failed, rescheduled, temporal);
    this.routeid = routeid;
  }

}

/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 10, 2002
 * Time: 4:38:25 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.smsstat;

import java.util.Vector;
import java.util.Collection;

public class RouteIdCountersSet extends CountersSet
{
  public String routeid;
  private Vector errors = new Vector(); // contains ErrorCounterSet

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

  public RouteIdCountersSet(CountersSet set, String routeid)
  {
    super(set);
    this.routeid = routeid;
  }

  public void addError(ErrorCounterSet set) {
    errors.addElement(set);
  }
  public Collection getErrors() {
    return errors;
  }
}

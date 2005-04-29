package ru.novosoft.smsc.admin.smsstat;

/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 10, 2002
 * Time: 4:38:25 PM
 * To change this template use Options | File Templates.
 */

import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.provider.Provider;

public class RouteIdCountersSet extends ExtendedCountersSet implements Comparable
{
  public String routeid;
  public long providerId = -1;
  public long categoryId = -1;

  public RouteIdCountersSet(String routeid)
  {
    this.routeid = routeid;
  }

  public RouteIdCountersSet(long accepted, long rejected, long delivered,
                            long failed, long rescheduled, long temporal, long i, long o, String routeid)
  {
    super(accepted, rejected, delivered, failed, rescheduled, temporal, i, o);
    this.routeid = routeid;
  }

  public RouteIdCountersSet(long accepted, long rejected, long delivered,
                             long failed, long rescheduled, long temporal, long i, long o, String routeid,
                            long providerId,long categoryId)
   {
     super(accepted, rejected, delivered, failed, rescheduled, temporal, i, o);
     this.routeid = routeid;
     this.providerId=providerId;
     this.categoryId=categoryId;
   }

  public void setCategoryId(long categoryId)
  {
    this.categoryId = categoryId;
  }
  public void setProviderId(long providerId)
  {
    this.providerId = providerId;
  }

  public int compareTo(Object o)
  {
    if (o == null || routeid == null || !(o instanceof RouteIdCountersSet)) return -1;
    return routeid.compareTo(((RouteIdCountersSet) o).routeid);
  }

}

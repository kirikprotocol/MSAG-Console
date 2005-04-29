package ru.novosoft.smsc.jsp.util.tables.impl.smcstat;

/**
 Created by andrey Date: 07.02.2005 Time: 12:45:18
 */

import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.admin.smsstat.RouteIdCountersSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

import java.util.Collection;

public class StatRouteDataItem extends AbstractDataItem
{

  public StatRouteDataItem(RouteIdCountersSet r)
  {
    values.put("Route ID", r.routeid);
    //values.put("providerId", new Long(r.providerId));
    //values.put("categoryId", new Long(r.categoryId));
    values.put("accepted", new Long(r.accepted));
    values.put("rejected", new Long(r.rejected));
    values.put("delivered", new Long(r.delivered));
    values.put("failed", new Long(r.failed));
    values.put("rescheduled", new Long(r.rescheduled));
    values.put("temporal", new Long(r.temporal));
    values.put("peak_i", new Long(r.peak_i));
    values.put("peak_o", new Long(r.peak_o));
    //values.put("providerName",r.providerName);
    //values.put("categoryName", r.categoryName);
    values.put("provider",new Long(r.providerId));
    values.put("category",new Long(r.categoryId));
    values.put("errors", r.getErrors());

  }

  public String getRouteID()
  {
    return (String) values.get("Route ID");
  }


  public long getAccepted()
  {
    return ((Long) values.get("accepted")).longValue();
  }


  public long getDelivered()
  {
    return ((Long) values.get("delivered")).longValue();
  }

  public long getFailed()
  {
    return ((Long) values.get("failed")).longValue();
  }

  public long getPeak_i()
  {
    return ((Long) values.get("peak_i")).longValue();
  }

  public long getPeak_o()
  {
    return ((Long) values.get("peak_o")).longValue();
  }


  public long getRejected()
  {
    return ((Long) values.get("rejected")).longValue();
  }

  public long getRescheduled()
  {
    return ((Long) values.get("rescheduled")).longValue();
  }

  public long getTemporal()
  {
    return ((Long) values.get("temporal")).longValue();
  }
 /*
  public Provider getProvider()
  {
    return (Provider) values.get("provider");
  }

  public Category getCategory()
  {
    return (Category) values.get("category");
  }
   */
  public Collection getErrors()
  {
    return (Collection) values.get("errors");
  }


  public long getProviderId()
  {
    return ((Long) values.get("providerId")).longValue();
  }
  public long getCategoryId()
  {
    return ((Long) values.get("categoryId")).longValue();
  }
  /*
   public String getProviderName()
  {
    return (String)values.get("providerName");
  }
  public String getCategoryName()
   {
     return (String)values.get("categoryName");
   }

  */
}

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
    values.put("accepted", new Integer(r.accepted));
    values.put("rejected", new Integer(r.rejected));
    values.put("delivered", new Integer(r.delivered));
    values.put("failed", new Integer(r.failed));
    values.put("rescheduled", new Integer(r.rescheduled));
    values.put("temporal", new Integer(r.temporal));
    values.put("peak_i", new Integer(r.peak_i));
    values.put("peak_o", new Integer(r.peak_o));
    //values.put("providerName",r.providerName);
    //values.put("categoryName", r.categoryName);
    values.put("provider", r.provider);
    values.put("category", r.category);
    values.put("errors", r.getErrors());

  }

  public String getRouteID()
  {
    return (String) values.get("Route ID");
  }


  public int getAccepted()
  {
    return ((Integer) values.get("accepted")).intValue();
  }


  public int getDelivered()
  {
    return ((Integer) values.get("delivered")).intValue();
  }

  public int getFailed()
  {
    return ((Integer) values.get("failed")).intValue();
  }

  public int getPeak_i()
  {
    return ((Integer) values.get("peak_i")).intValue();
  }

  public int getPeak_o()
  {
    return ((Integer) values.get("peak_o")).intValue();
  }


  public int getRejected()
  {
    return ((Integer) values.get("rejected")).intValue();
  }

  public int getRescheduled()
  {
    return ((Integer) values.get("rescheduled")).intValue();
  }

  public int getTemporal()
  {
    return ((Integer) values.get("temporal")).intValue();
  }

  public Provider getProvider()
  {
    return (Provider) values.get("provider");
  }

  public Category getCategory()
  {
    return (Category) values.get("category");
  }

  public Collection getErrors()
  {
    return (Collection) values.get("errors");
  }

  /*
  public Long getProviderId()
  {
    return (Long) values.get("providerId");
  }
  public Long getCategoryId()
  {
    return (Long) values.get("categoryId");
  }

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

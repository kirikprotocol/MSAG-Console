package ru.sibinco.smppgw.beans.routing.routes;

import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.smppgw.beans.TabledBeanImpl;

import java.util.Collection;


/**
 * Created by igork
 * Date: 19.04.2004
 * Time: 17:39:13
 */
public class Index extends TabledBeanImpl implements TabledBean
{
  protected Collection getDataSource()
  {
    return appContext.getGwRoutingManager().getRoutes().values();
  }

  protected void delete()
  {
    appContext.getGwRoutingManager().getRoutes().keySet().removeAll(checkedSet);
  }
}

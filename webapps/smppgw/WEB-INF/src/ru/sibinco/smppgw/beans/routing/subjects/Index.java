package ru.sibinco.smppgw.beans.routing.subjects;

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
    return appContext.getGwRoutingManager().getSubjects().values();
  }

  protected void delete()
  {
    appContext.getGwRoutingManager().getSubjects().keySet().removeAll(checkedSet);
  }
}

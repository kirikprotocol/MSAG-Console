package ru.sibinco.smppgw.beans.gw.users;

import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.smppgw.beans.TabledBeanImpl;

import java.util.Collection;


/**
 * Created by IntelliJ IDEA. User: igork Date: 03.03.2004 Time: 18:39:37
 */
public class Index extends TabledBeanImpl implements TabledBean
{
  protected Collection getDataSource()
  {
    return appContext.getUserManager().getUsers().values();
  }

  protected String getDefaultSort()
  {
    return "login";
  }

  protected void delete()
  {
    if (appContext.getUserManager().getUsers().keySet().removeAll(checkedSet))
      appContext.getStatuses().setUsersChanged(true);
  }
}

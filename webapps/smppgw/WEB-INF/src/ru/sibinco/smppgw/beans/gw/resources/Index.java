package ru.sibinco.smppgw.beans.gw.resources;

import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.smppgw.beans.SmppgwJspException;
import ru.sibinco.smppgw.beans.TabledBeanImpl;

import java.util.Collection;


/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 03.03.2004
 * Time: 18:39:37
 */
public class Index extends TabledBeanImpl implements TabledBean
{
  protected Collection getDataSource()
  {
    return appContext.getResourceManager().getResources().values();
  }

  protected void delete() throws SmppgwJspException
  {
  }

}

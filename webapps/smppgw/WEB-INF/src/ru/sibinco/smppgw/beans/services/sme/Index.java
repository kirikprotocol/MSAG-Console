package ru.sibinco.smppgw.beans.services.sme;

import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.smppgw.beans.TabledBeanImpl;

import java.util.Collection;


/**
 * Created by igork
 * Date: 17.03.2004
 * Time: 17:44:09
 */
public class Index extends TabledBeanImpl implements TabledBean
{
  protected Collection getDataSource()
  {
    return appContext.getGwSmeManager().getSmes().values();
  }

  protected void delete()
  {
    appContext.getGwSmeManager().getSmes().keySet().removeAll(checkedSet);
  }
}

package ru.sibinco.smppgw.beans.routing.billing;

import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.smppgw.beans.SmppgwJspException;
import ru.sibinco.smppgw.beans.TabledBeanImpl;

import java.util.Collection;


/**
 * Created by igork Date: 21.09.2004 Time: 17:55:57
 */
public class Index extends TabledBeanImpl implements TabledBean
{
  protected Collection getDataSource()
  {
    return appContext.getBillingManager().getRules().values();
  }

  protected void delete() throws SmppgwJspException
  {
    if (null != checkedSet && 0 < checkedSet.size()) {
      appContext.getBillingManager().getRules().keySet().removeAll(checkedSet);
      appContext.getStatuses().setBillingChanged(true);
    }
  }
}

package ru.sibinco.smppgw.beans.gw.smscs;

import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.smppgw.backend.sme.GwSme;
import ru.sibinco.smppgw.beans.TabledBeanImpl;

import java.util.Collection;
import java.util.Iterator;


/**
 * Created by igork Date: 22.03.2004 Time: 19:13:00
 */
public class Index extends TabledBeanImpl implements TabledBean
{
  protected Collection getDataSource()
  {
    return appContext.getSmscsManager().getSmscs().values();
  }

  protected void delete()
  {
    for (Iterator i = checkedSet.iterator(); i.hasNext();) {
      final String smscId = (String) i.next();
      final GwSme sme = (GwSme) appContext.getGwSmeManager().getSmes().get(smscId);
      if (null != sme)
        sme.setSmscInfo(null);
    }
    appContext.getSmscsManager().getSmscs().keySet().removeAll(checkedSet);
    appContext.getStatuses().setSmscsChanged(true);
  }
}

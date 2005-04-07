package ru.sibinco.smppgw.beans.gw.smscs;

import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.smppgw.backend.sme.GwSme;
import ru.sibinco.smppgw.backend.sme.SmscsManager;
import ru.sibinco.smppgw.beans.TabledBeanImpl;

import java.util.*;


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
    SmscsManager smscsManager=appContext.getSmscsManager();
    for (Iterator i = checkedSet.iterator(); i.hasNext();) {
      final String smscId = (String) i.next();
      final GwSme sme = (GwSme) appContext.getGwSmeManager().getSmes().get(smscId);
      if (null != sme)
        sme.setSmscInfo(null);
    }
    smscsManager.getSmscs().keySet().removeAll(checkedSet);
    smscsManager.setSmscsUnreg(checkedSet);
    appContext.getStatuses().setSmscsChanged(true);
  }
}

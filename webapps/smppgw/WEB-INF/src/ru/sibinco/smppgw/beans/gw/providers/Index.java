package ru.sibinco.smppgw.beans.gw.providers;

import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.smppgw.Constants;
import ru.sibinco.smppgw.backend.sme.GwSme;
import ru.sibinco.smppgw.beans.SmppgwJspException;
import ru.sibinco.smppgw.beans.TabledBeanImpl;

import java.util.*;


/**
 * Created by igork
 * Date: 30.03.2004
 * Time: 20:06:32
 */
public class Index extends TabledBeanImpl implements TabledBean
{
  protected Collection getDataSource()
  {
    return appContext.getProviderManager().getProviders().values();
  }

  protected void delete() throws SmppgwJspException
  {
    List toRemove = new ArrayList(checked.length);
    for (int i = 0; i < checked.length; i++) {
      String providerIdStr = checked[i];
      Long providerId = Long.decode(providerIdStr);
      toRemove.add(providerId);
    }

    Map smes = appContext.getGwSmeManager().getSmes();
    for (Iterator i = smes.values().iterator(); i.hasNext();) {
      GwSme sme = (GwSme) i.next();
      if (!sme.isSmsc() && toRemove.contains(new Long(sme.getProviderId())))
        throw new SmppgwJspException(Constants.errors.providers.COULDNT_DELETE_PROVIDER, sme.getProviderName());
    }

    appContext.getProviderManager().getProviders().keySet().removeAll(toRemove);
  }
}
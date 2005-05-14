package ru.sibinco.smppgw.beans.gw.smscs;

import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.protocol.Proxy;
import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.smppgw.backend.sme.GwSme;
import ru.sibinco.smppgw.backend.sme.SmscsManager;
import ru.sibinco.smppgw.backend.Gateway;
import ru.sibinco.smppgw.beans.TabledBeanImpl;
import ru.sibinco.smppgw.beans.SmppgwJspException;
import ru.sibinco.smppgw.Constants;

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

  protected void delete() throws SmppgwJspException
  {
    SmscsManager smscsManager=appContext.getSmscsManager();
    Gateway gateway=appContext.getGateway();
    try {
      for (Iterator i = checkedSet.iterator(); i.hasNext();) {
        final String smscId = (String) i.next();
        final GwSme sme = (GwSme) appContext.getGwSmeManager().getSmes().get(smscId);
        if (null != sme)
          sme.setSmscInfo(null);
        gateway.unregSmsc(smscId);
        smscsManager.getSmscs().remove(smscId);
      }
     appContext.getSmscsManager().storeSmscs(appContext.getGwConfig());
    } catch (SibincoException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
      if (Proxy.StatusConnected == appContext.getGateway().getStatus()) {
        logger.debug("Couldn't apply Service centers", e);
        throw new SmppgwJspException(Constants.errors.status.COULDNT_APPLY_SMSCS, e);
      }
    }
  }
}

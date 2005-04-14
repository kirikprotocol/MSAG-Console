package ru.sibinco.smppgw.beans.services.sme;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.protocol.Proxy;
import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.smppgw.Constants;
import ru.sibinco.smppgw.backend.Gateway;
import ru.sibinco.smppgw.backend.sme.GwSme;
import ru.sibinco.smppgw.beans.SmppgwJspException;
import ru.sibinco.smppgw.beans.TabledBeanImpl;

import java.util.*;


/**
 * Created by igork Date: 17.03.2004 Time: 17:44:09
 */
public class Index extends TabledBeanImpl implements TabledBean
{
  protected Collection getDataSource()
  {
    return appContext.getGwSmeManager().getSmes().values();
  }

  protected void delete() throws SmppgwJspException
  {
    final Gateway gateway = appContext.getGateway();
    final Map smes = appContext.getGwSmeManager().getSmes();
    final Map smscs = appContext.getSmscsManager().getSmscs();
    for (Iterator i = checkedSet.iterator(); i.hasNext();) {
      final String smeId = (String) i.next();
      try {
        GwSme sme= (GwSme)smes.get(smeId);
        if (sme.isSmsc()) {
         gateway.unregSmsc(smeId);
         smscs.remove(smeId);
        }
        gateway.deleteSme(smeId);
        smes.remove(smeId);
      } catch (SibincoException e) {
        if (Proxy.StatusConnected == gateway.getStatus()) {
          logger.error("Couldn't delete sme \"" + smeId + '"', e);
          throw new SmppgwJspException(Constants.errors.sme.COULDNT_DELETE, smeId, e);
        } else
          smes.remove(smeId);
      } finally {
        try {
          appContext.getGwSmeManager().store();
        } catch (SibincoException e) {
          logger.error("Couldn't store smes", e);
        }
         appContext.getSmscsManager().storeSmscs(appContext.getGwConfig());
      }
    }
  }
}

package ru.sibinco.scag.beans.services.sme;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.protocol.Proxy;
import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.Gateway;
import ru.sibinco.scag.backend.sme.GwSme;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.TabledBeanImpl;

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

  protected void delete() throws SCAGJspException
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
          throw new SCAGJspException(Constants.errors.sme.COULDNT_DELETE, smeId, e);
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

package ru.sibinco.smppgw.backend.sme;

import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.lib.backend.protocol.Proxy;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.smppgw.backend.Constants;
import ru.sibinco.smppgw.backend.Gateway;
import ru.sibinco.smppgw.backend.SmppGWAppContext;
import ru.sibinco.smppgw.beans.SmppgwJspException;

import java.util.*;

import org.apache.log4j.Logger;


/**
 * Created by igork Date: 30.03.2004 Time: 18:20:20
 */
public class SmscsManager
{
  private Map smscs = Collections.synchronizedMap(new HashMap());
  private static final String SECTION_NAME = "smsc-connections";
  protected Logger logger = Logger.getLogger(this.getClass());
  public SmscsManager(final Config gwConfig, final GwSmeManager smeManager) throws Config.WrongParamTypeException, Config.ParamNotFoundException
  { final Map smes=smeManager.getSmes();
    final Set smscIds = gwConfig.getSectionChildShortSectionNames(Constants.SECTION_NAME_SMSC_CONNECTIONS);
    for (Iterator i = smscIds.iterator(); i.hasNext();) {
      final String smscId = (String) i.next();
      final SmscInfo info = new SmscInfo(gwConfig, Constants.SECTION_NAME_SMSC_CONNECTIONS + '.' + smscId);
      smscs.put(smscId, info);
      if (smes.containsKey(smscId)) {
        final GwSme sme = (GwSme) smes.get(smscId);
        sme.setSmscInfo(info);
      }
    }
  }

  public Map getSmscs()
  {
    return smscs;
  }

   public void store(final Config gwConfig)
  {
    gwConfig.removeSection(SECTION_NAME);
    for (Iterator i = smscs.values().iterator(); i.hasNext();) {
      final SmscInfo smscInfo = (SmscInfo) i.next();
      smscInfo.store(gwConfig, SECTION_NAME); 
    }
  }
    public void storeSmscs(final Config gwConfig) throws SmppgwJspException
  {
    try {
      store(gwConfig);
      gwConfig.save();
    } catch (Throwable e) {
      logger.debug("Couldn't apply Service centers", e);
      throw new SmppgwJspException(ru.sibinco.smppgw.Constants.errors.status.COULDNT_APPLY_SMSCS, e);
    }
  }
}

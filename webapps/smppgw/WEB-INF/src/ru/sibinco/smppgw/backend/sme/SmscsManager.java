package ru.sibinco.smppgw.backend.sme;

import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.smppgw.backend.Constants;

import java.util.*;


/**
 * Created by igork Date: 30.03.2004 Time: 18:20:20
 */
public class SmscsManager
{
  private Map smscs = Collections.synchronizedMap(new HashMap());
  private static final String SECTION_NAME = "smsc-connections";

  public SmscsManager(final Config gwConfig) throws Config.WrongParamTypeException, Config.ParamNotFoundException
  {
    final Set smscIds = gwConfig.getSectionChildShortSectionNames(Constants.SECTION_NAME_SMSC_CONNECTIONS);
    for (Iterator i = smscIds.iterator(); i.hasNext();) {
      final String smscId = (String) i.next();
      final SmscInfo info = new SmscInfo(gwConfig, Constants.SECTION_NAME_SMSC_CONNECTIONS + '.' + smscId);
      smscs.put(smscId, info);
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
      smscInfo.store(gwConfig, SECTION_NAME); //todo: ??!
    }
  }
}


}

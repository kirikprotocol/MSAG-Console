package ru.sibinco.smppgw.backend.sme;

import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.smppgw.backend.Constants;
import ru.sibinco.smppgw.backend.Gateway;

import java.util.*;


/**
 * Created by igork Date: 30.03.2004 Time: 18:20:20
 */
public class SmscsManager
{
  private Map smscs = Collections.synchronizedMap(new HashMap());
  private Set smscsUnreg =new HashSet();
  private Set smscsNew =new HashSet();
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

  public Set getSmscsUnreg()
  {
    return smscsUnreg;
  }

  public void setSmscsUnreg(Set smscsUnreg)
  {
    this.smscsUnreg = smscsUnreg;
  }

  public Set getSmscsNew()
  {
    return smscsNew;
  }

  public void setSmscsNew(Set smscsNew)
  {
    this.smscsNew = smscsNew;
  }

   public void store(final Config gwConfig) throws SibincoException
  {
    gwConfig.removeSection(SECTION_NAME);
    for (Iterator i = smscs.values().iterator(); i.hasNext();) {
      final SmscInfo smscInfo = (SmscInfo) i.next();
      smscInfo.store(gwConfig, SECTION_NAME); 
    }
  }
  public void store(final Config gwConfig,final Gateway gateway) throws SibincoException
  {
    for (Iterator i = smscsUnreg.iterator(); i.hasNext();) {
      final String smscId = (String) i.next();
      gateway.unregSmsc(smscId);
    }
    smscsUnreg.clear();
    gwConfig.removeSection(SECTION_NAME);
    for (Iterator i = smscs.values().iterator(); i.hasNext();) {
      final SmscInfo smscInfo = (SmscInfo) i.next();
      smscInfo.store(gwConfig, SECTION_NAME);
      if (smscsNew.contains(smscInfo.getId()))
        gateway.regSmsc(smscInfo);
    }
    smscsNew.clear();
  }
}

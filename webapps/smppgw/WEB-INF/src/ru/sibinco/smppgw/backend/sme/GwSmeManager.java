package ru.sibinco.smppgw.backend.sme;

import org.apache.log4j.Logger;
import org.xml.sax.SAXException;
import ru.sibinco.lib.backend.sme.Sme;
import ru.sibinco.lib.backend.sme.SmeManager;
import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.smppgw.backend.Constants;

import javax.xml.parsers.ParserConfigurationException;
import java.io.IOException;
import java.util.*;


/**
 * Created by igork
 * Date: 24.03.2004
 * Time: 18:05:53
 */
public class GwSmeManager extends SmeManager
{
  private Logger logger = Logger.getLogger(this.getClass());

  public GwSmeManager(String configFilename, Config gwConfig)
      throws IOException, ParserConfigurationException, SAXException, Config.WrongParamTypeException, Config.ParamNotFoundException
  {
    super(configFilename);
    final Map smes = getSmes();
    final Map smscs = getSmscs(gwConfig);
    final List providers = getProviders(gwConfig);

    for (Iterator i = new Vector(smes.keySet()).iterator(); i.hasNext();) {
      String smeId = (String) i.next();
      Sme sme = (Sme) smes.get(smeId);
      SmscInfo smscInfo = (SmscInfo) smscs.get(smeId);
      if (smscInfo != null) {
        logger.debug("register SME \"" + smeId + "\" as SMSC");
        smes.put(smeId, new GwSme(sme, smscInfo));
      } else {
        logger.debug("register SME \"" + smeId + "\" as regular sme");
        smes.put(smeId, new GwSme(sme, (Provider) null)); //todo: reimplement
      }
    }
  }

  private List getProviders(Config gwConfig)
  {
    List results = new LinkedList();
    Set providersSections = gwConfig.getSectionChildShortSectionNames(Constants.SECTION_NAME_SME_PROVIDERS);
    for (Iterator i = providersSections.iterator(); i.hasNext();) {
      //todo: implement
    }
    return results;
  }

  private Map getSmscs(Config gwConfig) throws Config.WrongParamTypeException, Config.ParamNotFoundException
  {
    Map results = new HashMap();
    Set smscsSections = gwConfig.getSectionChildShortSectionNames(Constants.SECTION_NAME_SMSC_CONNECTIONS);
    for (Iterator i = smscsSections.iterator(); i.hasNext();) {
      String smscId = (String) i.next();
      results.put(smscId, new SmscInfo(gwConfig, Constants.SECTION_NAME_SMSC_CONNECTIONS + "." + smscId));
    }
    return results;
  }
}

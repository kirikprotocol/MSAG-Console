package ru.sibinco.smppgw.backend.sme;

import org.apache.log4j.Logger;
import org.w3c.dom.Element;
import org.xml.sax.SAXException;
import ru.sibinco.lib.backend.sme.Sme;
import ru.sibinco.lib.backend.sme.SmeManager;
import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.smppgw.backend.Constants;

import javax.xml.parsers.ParserConfigurationException;
import java.io.IOException;
import java.util.*;


/**
 * Created by igork Date: 24.03.2004 Time: 18:05:53
 */
public class GwSmeManager extends SmeManager
{
  private Logger logger = Logger.getLogger(this.getClass());
  private final ProviderManager providerManager;

  public GwSmeManager(final String configFilename, final Config gwConfig, final ProviderManager providerManager)
      throws IOException, ParserConfigurationException, SAXException, Config.WrongParamTypeException, Config.ParamNotFoundException
  {
    super(configFilename);
    this.providerManager = providerManager;
    final Map smes = getSmes();
    final Map smscs = getSmscs(gwConfig);
    //final Map providers = providerManager.getProviders();

    for (Iterator i = new ArrayList(smes.keySet()).iterator(); i.hasNext();) {
      final String smeId = (String) i.next();
      final Sme sme = (Sme) smes.get(smeId);
      final SmscInfo smscInfo = (SmscInfo) smscs.get(smeId);
      if (null != smscInfo) {
        logger.debug("register SME \"" + smeId + "\" as SMSC");
        smes.put(smeId, new GwSme(sme, smscInfo));
      } else {
        logger.debug("register SME \"" + smeId + "\" as regular sme");
        smes.put(smeId, new GwSme(sme, (Provider) null)); //todo: reimplement
      }
    }
  }

  protected Sme createSme(final Element smeRecord)
  {
    return new GwSme(smeRecord, providerManager);
  }

  private Map getSmscs(final Config gwConfig) throws Config.WrongParamTypeException, Config.ParamNotFoundException
  {
    final Map results = new HashMap();
    final Set smscsSections = gwConfig.getSectionChildShortSectionNames(Constants.SECTION_NAME_SMSC_CONNECTIONS);
    for (Iterator i = smscsSections.iterator(); i.hasNext();) {
      final String smscId = (String) i.next();
      results.put(smscId, new SmscInfo(gwConfig, Constants.SECTION_NAME_SMSC_CONNECTIONS + "." + smscId));
    }
    return results;
  }
}

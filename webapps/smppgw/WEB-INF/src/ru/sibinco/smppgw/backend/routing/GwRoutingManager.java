package ru.sibinco.smppgw.backend.routing;

import org.apache.log4j.Logger;
import org.w3c.dom.Element;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.route.*;
import ru.sibinco.lib.backend.sme.SmeManager;
import ru.sibinco.smppgw.backend.sme.ProviderManager;

import java.io.File;
import java.util.Map;


/**
 * Created by igork Date: 19.04.2004 Time: 17:41:55
 */
public class GwRoutingManager extends RouteSubjectManager
{
  private final Logger logger = Logger.getLogger(this.getClass());
  private final ProviderManager providerManager;

  public GwRoutingManager(final File smscConfFolder, final SmeManager smeManager, final ProviderManager providerManager)
      throws SibincoException
  {
    super(smscConfFolder, smeManager);
    this.providerManager = providerManager;
  }

  public void init() throws SibincoException
  {
    super.init();
  }

  protected Subject createSubject(final Element subjElem, final SmeManager smeManager) throws SibincoException
  {
    return new Subject(subjElem, smeManager);
  }

  protected Route createRoute(final Element routeElem, final Map subjects, final SmeManager smeManager) throws SibincoException
  {
    return new GwRoute(routeElem, subjects, smeManager, providerManager);
  }
}

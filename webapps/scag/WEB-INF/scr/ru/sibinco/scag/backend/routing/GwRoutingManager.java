package ru.sibinco.scag.backend.routing;

import org.w3c.dom.Element;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.route.*;
import ru.sibinco.lib.backend.sme.SmeManager;
import ru.sibinco.scag.backend.sme.ProviderManager;

import java.io.File;
import java.util.Map;


/**
 * Created by igork Date: 19.04.2004 Time: 17:41:55
 */
public class GwRoutingManager extends RouteSubjectManager
{
  private final ProviderManager providerManager;
  private final BillingManager billingManager;

  public GwRoutingManager(final File smscConfFolder, final SmeManager smeManager, final ProviderManager providerManager, final BillingManager billingManager)
  {
    super(smscConfFolder, smeManager);
    this.providerManager = providerManager;
    this.billingManager = billingManager;
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
    return new GwRoute(routeElem, subjects, smeManager, providerManager, billingManager);
  }
}

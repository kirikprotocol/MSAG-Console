package ru.sibinco.smppgw.backend.routing;

import org.w3c.dom.Element;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.route.Route;
import ru.sibinco.lib.backend.sme.SmeManager;
import ru.sibinco.smppgw.backend.sme.Provider;

import java.util.Map;


/**
 * Created by igork
 * Date: 19.04.2004
 * Time: 19:05:23
 */
public class GwRoute extends Route
{
  private Provider provider;
  private TrafficRules trafficRules;

  public GwRoute(Element routeElem, Map subjects, SmeManager smeManager, Provider provider)
      throws SibincoException
  {
    super(routeElem, subjects, smeManager);
    this.provider = provider;
    this.trafficRules = new TrafficRules(routeElem.getAttribute("trafficRules"));
  }

  public GwRoute(String routeName, int priority, boolean isEnabling, boolean isBilling, boolean isArchiving, boolean isSuppressDeliveryReports, boolean active, int serviceId, Map sources, Map destinations, String srcSmeId, String deliveryMode, String forwardTo, boolean hide, boolean forceReplayPath, String notes, Provider provider, TrafficRules trafficRules)
  {
    super(routeName, priority, isEnabling, isBilling, isArchiving, isSuppressDeliveryReports, active, serviceId, sources, destinations, srcSmeId, deliveryMode, forwardTo, hide, forceReplayPath, notes);
    this.provider = provider;
    this.trafficRules = trafficRules;
  }

  public Provider getProvider()
  {
    return provider;
  }

  public void setProvider(Provider provider)
  {
    this.provider = provider;
  }

  public String getProviderName()
  {
    if (provider != null)
      return provider.getName();
    else
      return null;
  }

  public TrafficRules getTrafficRules()
  {
    return trafficRules;
  }

  public void setTrafficRules(TrafficRules trafficRules)
  {
    this.trafficRules = trafficRules;
  }
}

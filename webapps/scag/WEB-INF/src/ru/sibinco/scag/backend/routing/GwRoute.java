package ru.sibinco.scag.backend.routing;

import org.w3c.dom.Element;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.route.*;
import ru.sibinco.lib.backend.sme.SmeManager;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.backend.sme.ProviderManager;

import java.io.PrintWriter;
import java.util.Iterator;
import java.util.Map;


/**
 * Created by igork Date: 19.04.2004 Time: 19:05:23
 */
public class GwRoute extends Route
{
  private static final String BILLING_ID_ATTRIBUTE = "billingId";
  private static final String PROVIDER_ID_ATTRIBUTE = "providerId";
  private Provider provider;
  private TrafficRules trafficRules;
  private String billingRuleId;

  public GwRoute(final Element routeElem, final Map subjects, final SmeManager smeManager, final ProviderManager providerManager,
                 final BillingManager billingManager)
      throws SibincoException
  {
    super(routeElem, subjects, smeManager);
    this.billingRuleId = "";
    if (routeElem.hasAttribute(PROVIDER_ID_ATTRIBUTE)) {
      final Long providerId = Long.decode(routeElem.getAttribute(PROVIDER_ID_ATTRIBUTE));
      this.provider = (Provider) providerManager.getProviders().get(providerId);
    }
    this.trafficRules = new TrafficRules(routeElem.getAttribute("trafficRules"));
    if (routeElem.hasAttribute(BILLING_ID_ATTRIBUTE)) {
      this.billingRuleId = routeElem.getAttribute(BILLING_ID_ATTRIBUTE);
    }
  }

  public GwRoute(final String routeName, final int priority, final boolean isEnabling, final boolean active, final int serviceId, final Map sources,
                 final Map destinations, final String srcSmeId, final String notes, final Provider provider, final TrafficRules trafficRules,
                 final String billingRuleId)
  {
    super(routeName, priority, isEnabling, false, false, false, active, serviceId, sources, destinations, srcSmeId, "default", "", false, false, notes);
    this.billingRuleId = billingRuleId;
    this.provider = provider;
    this.trafficRules = trafficRules;
  }

  public Provider getProvider()
  {
    return provider;
  }

  public void setProvider(final Provider provider)
  {
    this.provider = provider;
  }

  public String getProviderName()
  {
    if (null != provider)
      return provider.getName();
    else
      return null;
  }

  public TrafficRules getTrafficRules()
  {
    return trafficRules;
  }

  public void setTrafficRules(final TrafficRules trafficRules)
  {
    this.trafficRules = trafficRules;
  }

  public PrintWriter store(final PrintWriter out)
  {
    out.println("  <route id=\"" + StringEncoderDecoder.encode(getName()) + "\" billing=\"" + isBilling()
                + "\" archiving=\"" + isArchiving() + "\" enabling=\"" + isEnabling() + "\" priority=\"" + getPriority()
                + "\" serviceId=\"" + getServiceId() + "\" suppressDeliveryReports=\"" + isSuppressDeliveryReports()
                + "\" active=\"" + isActive() + "\" srcSmeId=\"" + StringEncoderDecoder.encode(getSrcSmeId())
                + "\" deliveryMode=\"" + StringEncoderDecoder.encode(getDeliveryMode())
                + "\" hide=\"" + (isHide() ? "true" : "false")
                + "\" forceReplayPath=\"" + (isForceReplayPath() ? "true" : "false")
                + ("MAP_PROXY".equals(getSrcSmeId()) ? "\" forwardTo=\"" + StringEncoderDecoder.encode(getForwardTo()) : "")
                + (null != provider ? "\" providerId=\"" + provider.getId() : "")
                + (null != trafficRules ? "\" trafficRules=\"" + trafficRules.getText() : "")
                + (null != billingRuleId && 0 != billingRuleId.length() ? "\" billingId=\"" + billingRuleId : "")
                + "\">");
    if (null != getNotes())
      out.println("    <notes>" + getNotes() + "</notes>");

    for (Iterator i = getSources().values().iterator(); i.hasNext();) {
      final Source source = (Source) i.next();
      source.store(out);
    }
    for (Iterator i = getDestinations().values().iterator(); i.hasNext();) {
      final Destination destination = (Destination) i.next();
      destination.store(out);
    }
    out.println("  </route>");
    return out;
  }

  public String getBillingRuleId()
  {
    return billingRuleId;
  }

  public void setBillingRuleId(String billingRuleId)
  {
    this.billingRuleId = billingRuleId;
  }
}

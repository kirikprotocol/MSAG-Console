/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 19, 2002
 * Time: 6:07:20 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.route;

import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.admin.route.Destination;
import ru.novosoft.smsc.admin.route.Route;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.admin.route.Source;

import java.util.Iterator;


public class RouteViewCommand extends CommandClass
{
  private String route;

  public void setRoute(String route)
  {
    this.route = route;
  }

  private String quoteString(String str)
  {
    return (str != null &&
            (str.indexOf(' ') != -1 ||
             str.indexOf('\t') != -1 ||
             str.indexOf('\f') != -1)) ? "'" + str + "'" : str;
  }

  private String trafficModeToString(int trafficMode) {
    switch (trafficMode) {
      case Route.TRAFFIC_MODE_ALL: return "any traffic";
      case Route.TRAFFIC_MODE_SMS: return "sms only traffic";
      case Route.TRAFFIC_MODE_USSD: return "ussd only traffic";
      default:
        return "traffic prohibited";
    }
  }

  private void viewRoute(Route smscRoute, CommandContext ctx)
  {
    ctx.setMessage("Route info");
    ctx.setStatus(CommandContext.CMD_LIST);

    long aclId = smscRoute.getAclId();
    ctx.addResult("name  : " + smscRoute.getName());
    ctx.addResult("prio  : " + smscRoute.getPriority());
    ctx.addResult("svcid : " + smscRoute.getServiceId());
    ctx.addResult("srcsme: " + smscRoute.getSrcSmeId());
    ctx.addResult("dm    : " + smscRoute.getDeliveryMode());
    ctx.addResult("fwd   : " + smscRoute.getForwardTo());
    ctx.addResult("acl   : " + ((aclId <= -1) ? "-":""+aclId));
    ctx.addResult("notes : " + smscRoute.getNotes());
    ctx.addResult("flags : " +
                  (smscRoute.isActive() ? "active, " : "inactive, ") +
                  (smscRoute.isHide() ? "hide, " : "nohide, ") +
                  "replayPath " + Route.getReplayPathValue(smscRoute.getReplayPath()) +
                  "forceDelivery " + (smscRoute.isForceDelivery() ? "on, " : "off, ") +
                  "transit "+(smscRoute.isTransit() ? "on, " : "off, ") +
                  "blocked "+(smscRoute.isAllowBlocked() ? "allowed, ":"denied, ") +
                  "billing "+smscRoute.getBillingString()+", " +
                  (smscRoute.isArchiving() ? "archiving, " : "no archiving, ") +
                  trafficModeToString(smscRoute.getTrafficMode()) +
                  (smscRoute.isSuppressDeliveryReports() ? "no receipt" : "receipt"));
    Provider provider = ctx.getProviderManager().getProvider(new Long(smscRoute.getProviderId()));
    ctx.addResult("provider: " + (provider != null ? provider.getName():"-") +
                                           " (" + smscRoute.getProviderId() + ")");
    Category category = ctx.getCategoryManager().getCategory(new Long(smscRoute.getCategoryId()));
    ctx.addResult("category: " + (category != null ? category.getName():"-") +
                                           " (" + smscRoute.getCategoryId() + ")");

    String srcsStr = "";
    Iterator srcs = smscRoute.getSources().iterator();
    while (srcs.hasNext()) {
      Source src = (Source) srcs.next();
      srcsStr += quoteString(src.getName());
      if (srcs.hasNext()) srcsStr += ", ";
    }
    ctx.addResult("srcs : " + srcsStr);

    String dstsStr = "";
    Iterator dsts = smscRoute.getDestinations().iterator();
    while (dsts.hasNext()) {
      Destination dst = (Destination) dsts.next();
      dstsStr += quoteString(dst.getName());
      SME sme = dst.getSme();
      if (sme != null) {
        String smeId = sme.getId();
        if (smeId != null) dstsStr += " (" + smeId + ")";
      }
      if (dsts.hasNext()) dstsStr += ", ";
    }
    ctx.addResult("dsts : " + dstsStr);
  }

  public void process(CommandContext ctx)
  {
    String out = "Route '" + route + "'";
    try {
      Route smscRoute = ctx.getRouteSubjectManager().getRoutes().get(route);
      if (smscRoute == null) {
        ctx.setMessage(out + " not found");
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return;
      }
      viewRoute(smscRoute, ctx);
    } catch (Exception e) {
      ctx.setMessage("Couldn't view " + out + ". Cause: " + e.getMessage());
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
      return;
    }
  }

  public String getId()
  {
    return "ROUTE_VIEW";
  }

}

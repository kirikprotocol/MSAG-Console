/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 19, 2002
 * Time: 6:07:20 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.route.Route;
import ru.novosoft.smsc.admin.route.Source;
import ru.novosoft.smsc.admin.route.Destination;

import java.util.Iterator;

public class RouteViewCommand implements Command
{
    private String route;

    public void setRoute(String route) {
        this.route = route;
    }

    private String quoteString(String str) {
        return (str != null &&
                (str.indexOf(' ') != -1  ||
                 str.indexOf('\t') != -1 ||
                 str.indexOf('\f') != -1)) ? "'"+str+"'" : str;
    }
    private void viewRoute(Route smscRoute, CommandContext ctx)
    {
        ctx.setMessage("Route info");
        ctx.setStatus(CommandContext.CMD_LIST);

        ctx.addResult("name : "+smscRoute.getName());
        ctx.addResult("prio : "+smscRoute.getPriority());
        ctx.addResult("svcid: "+smscRoute.getServiceId());
        ctx.addResult("flags: "+
                        (smscRoute.isArchiving() ? "archiving, ": "no archiving, ")+
                        (smscRoute.isBilling()   ? "billing, ":"no billing, ")+
                        (smscRoute.isEnabling()  ? "allowed":"denied"));

        String srcsStr = "";
        Iterator srcs = smscRoute.getSources().iterator();
        while (srcs.hasNext()) {
            Source src = (Source)srcs.next();
            srcsStr += quoteString(src.getName());
            if (srcs.hasNext()) srcsStr += ", ";
        }
        ctx.addResult("srcs : "+srcsStr);

        String dstsStr = "";
        Iterator dsts = smscRoute.getDestinations().iterator();
        while (dsts.hasNext()) {
            Destination dst = (Destination)dsts.next();
            dstsStr += quoteString(dst.getName());
            String smeId = dst.getSme().getId();
            if (smeId != null) dstsStr += " ("+smeId+")";
            if (dsts.hasNext()) dstsStr += ", ";
        }
        ctx.addResult("dsts : "+dstsStr);
    }

    public void process(CommandContext ctx)
    {
        String out = "Route '"+route+"'";
        try
        {
            Route smscRoute = ctx.getSmsc().getRoutes().get(route);
            if (smscRoute == null) {
                ctx.setMessage(out+" not found");
                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                return;
            }
            viewRoute(smscRoute, ctx);
        }
        catch (Exception e) {
            ctx.setMessage("Couldn't view "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            return;
        }
    }

    public String getId() {
        return "ROUTE_VIEW";
    }

}

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
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.route.Destination;

import java.util.Iterator;

public class RouteViewCommand implements Command
{
    private String route;

    public void setRoute(String route) {
        this.route = route;
    }

    private String showRoute(Route smscRoute)
    {
        String out = "Route '"+smscRoute.getName()+"' Priority: "+smscRoute.getPriority()+
                     " SvcId: "+smscRoute.getServiceId()+" Options ["+
                     (smscRoute.isArchiving() ? "Archiving, ": "No arciving, ")+
                     (smscRoute.isBilling()   ? "Billing, ":"No billing, ")+
                     (smscRoute.isEnabling()  ? "Allowed":"Denied")+"]";

        Iterator srcs = smscRoute.getSources().iterator();
        if (srcs.hasNext()) out += " Srcs: ";
        while (srcs.hasNext()) {
            Source src = (Source)srcs.next();
            out += "<"+src.getName()+" DefSME:"+src.getDefaultSmeId();
            Iterator masks = src.getMasks().iterator();
            out += " Mask(s): ";
            while (masks.hasNext()) {
                out += ((Mask)masks.next()).getMask();
                if (masks.hasNext()) out += ", ";
            }
            out += srcs.hasNext() ? ">, ":">";
        }

        Iterator dsts = smscRoute.getDestinations().iterator();
        if (dsts.hasNext()) out += " Dsts: ";
        while (dsts.hasNext()) {
            Destination dst = (Destination)dsts.next();
            out += "<"+dst.getName()+" SME: "+dst.getSme().getId()+
                    " DefSME:"+dst.getDefaultSmeId();
            Iterator masks = dst.getMasks().iterator();
            out += " Mask(s): ";
            while (masks.hasNext()) {
                out += ((Mask)masks.next()).getMask();
                if (masks.hasNext()) out += ", ";
            }
            out += dsts.hasNext() ? ">, ":">";
        }
        return out;
    }

    public void process(CommandContext ctx)
    {
        String out = "Route '"+route+"' ";
        try
        {
            Route smscRoute = ctx.getSmsc().getRoutes().get(route);
            if (smscRoute == null) {
                ctx.setMessage(out+"not exists");
                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                return;
            }
            ctx.setMessage(showRoute(smscRoute));
            ctx.setStatus(CommandContext.CMD_OK);
        }
        catch (Exception e) {
            ctx.setMessage("Failed to view "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            return;
        }
    }
}

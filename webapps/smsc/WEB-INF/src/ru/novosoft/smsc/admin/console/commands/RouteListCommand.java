/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 19, 2002
 * Time: 6:05:57 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.route.Route;

import java.util.Iterator;

public class RouteListCommand implements Command
{
    public void process(CommandContext ctx)
    {
        try
        {
            Iterator i = ctx.getRouteSubjectManager().getRoutes().iterator();
            if (!i.hasNext()) {
                ctx.setMessage("No routes defined");
                ctx.setStatus(CommandContext.CMD_OK);
            }
            else {
                while (i.hasNext()) {
                    Route route = (Route)i.next();
                    if (route != null) {
                        ctx.addResult(route.getName());
                    }
                }
                ctx.setMessage("Routes list");
                ctx.setStatus(CommandContext.CMD_LIST);
            }
        }
        catch (Exception e) {
            ctx.setMessage("Couldn't list routes. Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            return;
        }
    }

    public String getId() {
        return "ROUTE_LIST";
    }

}

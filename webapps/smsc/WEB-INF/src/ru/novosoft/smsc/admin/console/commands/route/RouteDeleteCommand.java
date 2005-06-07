/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:09:06 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.route;


import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.route.Route;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.journal.Actions;

public class RouteDeleteCommand extends CommandClass
{
    private String route;

    public void setRoute(String route) {
        this.route = route;
    }

    public void process(CommandContext ctx)
    {
        String out = "Route '"+route+"'";
        try {
            Route oldRoute = ctx.getRouteSubjectManager().getRoutes().remove(route);
            if (oldRoute == null) {
                ctx.setMessage(out+" not found");
                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                return;
            }
        }
        catch (Exception e) {
            ctx.setMessage("Couldn't delete "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            return;
        }

        ctx.setMessage(out+" deleted");
        ctx.setStatus(CommandContext.CMD_OK);
    }

    public String getId() {
        return "ROUTE_DELETE";
    }

	public void updateJournalAndStatuses(CommandContext ctx, String userName)
	{
		journalAppend(ctx, userName, SubjectTypes.TYPE_route, route, Actions.ACTION_DEL);
		ctx.getStatuses().setRoutesChanged(true);
	}
}


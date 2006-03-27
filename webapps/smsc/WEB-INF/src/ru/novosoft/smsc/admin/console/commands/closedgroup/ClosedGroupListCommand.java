package ru.novosoft.smsc.admin.console.commands.closedgroup;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.closedgroups.ClosedGroup;

import java.util.Iterator;

/**
 * Created by IntelliJ IDEA.
 * User: starkom
 * Date: 26.02.2006
 * Time: 16:05:45
 * To change this template use File | Settings | File Templates.
 */
public class ClosedGroupListCommand extends ClosedGroupGenCommand{
    public void process(CommandContext ctx)
    {
        try
        {
            Iterator i = ctx.getClosedGroupManager().getClosedGroups().iterator();
            if (!i.hasNext()) {
                ctx.setMessage("No closed groups found");
                ctx.setStatus(CommandContext.CMD_OK);
            } else {
                while (i.hasNext()) {
                    String cgName = ((ClosedGroup)i.next()).getName();
                    if (cgName != null && cgName.length() > 0) ctx.addResult(cgName);
                }
                ctx.setMessage("closed group list");
                ctx.setStatus(CommandContext.CMD_LIST);
            }
        }
        catch (Exception e) {
            ctx.setMessage("Couldn't list closed groups. Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            return;
        }
    }

    public String getId() {
        return "CLOSEDGROUP_LIST";
    }
}

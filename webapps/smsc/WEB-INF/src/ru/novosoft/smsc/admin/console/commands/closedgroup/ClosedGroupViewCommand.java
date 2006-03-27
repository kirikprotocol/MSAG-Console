package ru.novosoft.smsc.admin.console.commands.closedgroup;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.closedgroups.ClosedGroup;

/**
 * Created by IntelliJ IDEA.
 * User: starkom
 * Date: 26.02.2006
 * Time: 15:47:03
 * To change this template use File | Settings | File Templates.
 */
public class ClosedGroupViewCommand extends ClosedGroupGenCommand {
    public void process(CommandContext ctx) {
        super.process(ctx);
        String out = "closed group '" + groupIdOrName + "'";
        try {
            ClosedGroup cg = ctx.getClosedGroupManager().getClosedGroups().get(groupName);
            if (cg != null) {
                ctx.setMessage("closed group info");
                ctx.setStatus(CommandContext.CMD_LIST);
                ctx.addResult("id: " + cg.getId());
                ctx.addResult("name: " + cg.getName());
                ctx.addResult("description: " + cg.getDef());
                ctx.addResult("masks: " + cg.getMasks().getText());
            } else {
                ctx.setMessage(out + " not found");
                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            }
        } catch (Exception e) {
            ctx.setMessage("Couldn't view " + out + ". Cause: " + e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "CLOSEDGROUP_VIEW";
    }
}

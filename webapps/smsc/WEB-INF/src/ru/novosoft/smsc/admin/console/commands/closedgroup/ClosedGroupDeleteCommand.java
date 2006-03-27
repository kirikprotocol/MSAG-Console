package ru.novosoft.smsc.admin.console.commands.closedgroup;

import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.journal.Actions;

/**
 * Created by IntelliJ IDEA.
 * User: starkom
 * Date: 26.02.2006
 * Time: 15:44:18
 * To change this template use File | Settings | File Templates.
 */
public class ClosedGroupDeleteCommand extends ClosedGroupGenCommand {
    public void process(CommandContext ctx)
    {
        super.process(ctx);
        String out = "closed group '" + groupIdOrName + "' ";
        try {
            ctx.getClosedGroupManager().removeGroup(groupName);
            ctx.setMessage(out + " deleted");
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (Exception e) {
            ctx.setMessage(out + " not deleted. Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "CLOSEDGROUP_DELETE";
    }

    public void updateJournalAndStatuses(CommandContext ctx, String userName)
    {
        journalAppend(ctx, userName, SubjectTypes.TYPE_closedgroup, groupName, Actions.ACTION_DEL);
        ctx.getStatuses().setClosedGroupChanged(true);
    }
}

package ru.novosoft.smsc.admin.console.commands.closedgroup;

import ru.novosoft.smsc.admin.closedgroups.ClosedGroup;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;

public class ClosedGroupAddCommand extends ClosedGroupGenCommand {
    public void process(CommandContext ctx) {
        super.process(ctx);
        String out = "closed group  '" + groupIdOrName + "'";

        try {
            ClosedGroup cg = new ClosedGroup(groupName, descr, new String[0]);
            ctx.getClosedGroupManager().add(cg);
            ctx.setMessage(out + " added");
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (Exception e) {
            ctx.setMessage("Couldn't add " + out + ". Cause: " + e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "CLOSEDGROUP_ADD";
    }

    public void updateJournalAndStatuses(CommandContext ctx, String userName) {
        journalAppend(ctx, userName, SubjectTypes.TYPE_closedgroup, groupName, Actions.ACTION_ADD);
        ctx.getStatuses().setClosedGroupChanged(true);
    }
}

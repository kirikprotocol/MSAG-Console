package ru.novosoft.smsc.admin.console.commands.closedgroup;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.closedgroups.ClosedGroup;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.route.MaskList;

/**
 * Created by IntelliJ IDEA.
 * User: starkom
 * Date: 26.02.2006
 * Time: 16:08:46
 * To change this template use File | Settings | File Templates.
 */
public class ClosedGroupAlterCommand extends ClosedGroupGenCommand {
    private MaskList masks2Add = new MaskList();
    private MaskList masks2Del = new MaskList();
    private boolean isMasks = false;
    private String newName = "";
    private boolean isNewName = false;

    public void process(CommandContext ctx) {
        super.process(ctx);
        String out = "closed group '" + groupIdOrName + "'";
        try {
            ClosedGroup cg = ctx.getClosedGroupManager().getClosedGroups().get(groupName);
            if (cg == null) {
                ctx.setMessage(out + " not found");
                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            } else {
                if (isDescr || isNewName || isMasks)
                    ctx.getClosedGroupManager().alter(cg, newName, descr, masks2Add, masks2Del);
                ctx.setMessage(out + " altered");
                ctx.setStatus(CommandContext.CMD_OK);
            }
        } catch (Exception e) {
            ctx.setMessage("Couldn't alter " + out + ". Cause: " + e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "CLOSEDGROUP_ALTER";
    }

    public void updateJournalAndStatuses(CommandContext ctx, String userName) {
        journalAppend(ctx, userName, SubjectTypes.TYPE_closedgroup, groupName, Actions.ACTION_MODIFY);
        ctx.getStatuses().setClosedGroupChanged(true);
    }

    public void addMask(String mask) throws AdminException {
        Mask newMask = new Mask(mask);
        masks2Add.add(newMask);
        isMasks = true;
    }

    public void delMask(String mask) throws AdminException {
        Mask newMask = new Mask(mask);
        masks2Del.add(newMask);
        isMasks = true;
    }

    public void setClosedGroupNewName(String name) {
        newName = name;
        isNewName = true;
    }
}

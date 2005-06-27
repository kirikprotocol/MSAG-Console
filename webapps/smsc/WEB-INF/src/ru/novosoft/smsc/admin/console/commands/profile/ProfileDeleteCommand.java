/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:58:28 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.profile;


import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.CommandClass;

import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;

public class ProfileDeleteCommand extends CommandClass
{
    private String mask;

    public void setMask(String mask) {
        this.mask = mask;
    }

    public void process(CommandContext ctx)
    {
        String out = "Profile for mask '"+mask+"' ";
        try {
            ctx.getSmsc().profileDelete(new Mask(mask));
            ctx.setMessage(out+"deleted");
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (Exception e) {
            ctx.setMessage(out+"not deleted. Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "PROFILE_DELETE";
    }

	public void updateJournalAndStatuses(CommandContext ctx, String userName)
	{
		journalAppend(ctx, userName, SubjectTypes.TYPE_profile, mask, Actions.ACTION_DEL);
		ctx.getStatuses().setProfilesChanged(true);
	}
}


package ru.novosoft.smsc.admin.console.commands.sme;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 11.04.2005
 * Time: 17:37:23
 * To change this template use File | Settings | File Templates.
 */
public class SmeDeleteCommand extends SmeGenCommand
{
    public void process(CommandContext ctx)
    {
        String out = "SME '"+smeId+"'";
        try {
            SmeManager manager = ctx.getSmeManager();
            if (!manager.contains(smeId)) throw new Exception("SME not exists");
            manager.remove(smeId);
            ctx.setMessage(out+" deleted");
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (Exception e) {
            ctx.setMessage("Couldn't delete "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "SME_DELETE";
    }

	public void updateJournalAndStatuses(CommandContext ctx, String userName)
	{
		journalAppend(ctx, userName, SubjectTypes.TYPE_service, smeId, Actions.ACTION_DEL);
	}
}

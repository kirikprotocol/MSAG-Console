package ru.novosoft.smsc.admin.console.commands.apply;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.journal.SubjectTypes;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 26.05.2005
 * Time: 16:17:09
 * To change this template use File | Settings | File Templates.
 */
public class ApplyCategoriesCommand extends CommandClass
{
    public void process(CommandContext ctx)
    {
        try {
            ctx.getCategoryManager().apply();
            ctx.setMessage("Categories applied succesfully");
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (Exception e) {
            ctx.setMessage("Couldn't apply categories changes. Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            e.printStackTrace();
        }
    }

    public String getId() {
        return "CATEGORIES_APPLY";
    }

	public void updateJournalAndStatuses(CommandContext ctx, String userName)
	{
		ctx.getStatuses().setCategoriesChanged(false);
		ctx.getJournal().clear(SubjectTypes.TYPE_category);
	}
}

/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:24:53 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.apply;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.journal.SubjectTypes;

public class ApplyAllCommand extends CommandClass
{
    public void process(CommandContext ctx)
    {
        try {
            ctx.getSmsc().applyProfiles();
            ctx.getSmsc().applyAliases();
            ctx.getCategoryManager().apply();
            ctx.getProviderManager().apply();
            ctx.getSmsc().applyRoutes(ctx.getRouteSubjectManager());
            ctx.setMessage("All changes applied succesfully");
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (Exception e) {
            ctx.setMessage("Couldn't apply all changes. Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            e.printStackTrace();
        }
    }

    public String getId() {
        return "APPLY_ALL";
    }

	public void updateJournalAndStatuses(CommandContext ctx, String userName)
	{
		ctx.getStatuses().setRoutesChanged(false);
		ctx.getJournal().clear(SubjectTypes.TYPE_route);
		ctx.getStatuses().setCategoriesChanged(false);
		ctx.getJournal().clear(SubjectTypes.TYPE_category);
		ctx.getStatuses().setProvidersChanged(false);
		ctx.getJournal().clear(SubjectTypes.TYPE_provider);
		ctx.getStatuses().setAliasesChanged(false);
		ctx.getJournal().clear(SubjectTypes.TYPE_alias);
		ctx.getStatuses().setProfilesChanged(false);
		ctx.getJournal().clear(SubjectTypes.TYPE_profile);
	}
}


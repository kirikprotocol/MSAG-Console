package ru.novosoft.smsc.admin.console.commands.misc;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.provider.ProviderManager;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.journal.Actions;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 12.04.2005
 * Time: 14:44:45
 * To change this template use File | Settings | File Templates.
 */

public class ProviderDeleteCommand extends CommandClass
{
    protected String providerId = null;

    public void process(CommandContext ctx)
    {
        String out = "Provider '"+providerId+"'";
        try {
            ProviderManager manager = ctx.getProviderManager();
            Provider provider = manager.getProviderByName(providerId);
            if (provider == null) throw new Exception("Provider not exists");
            manager.removeProvider(providerId);
            ctx.setMessage(out+" deleted");
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (Exception e) {
            ctx.setMessage("Couldn't delete "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public void setProviderId(String providerId) {
        this.providerId = providerId;
    }

    public String getId() {
        return "PROVIDER_DELETE";
    }

	public void updateJournalAndStatuses(CommandContext ctx, String userName)
	{
		journalAppend(ctx, userName, SubjectTypes.TYPE_provider, providerId, Actions.ACTION_DEL);
		ctx.getStatuses().setProvidersChanged(true);
	}
}

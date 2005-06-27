package ru.novosoft.smsc.admin.console.commands.misc;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.provider.ProviderManager;
import ru.novosoft.smsc.admin.provider.Provider;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 12.04.2005
 * Time: 14:42:13
 * To change this template use File | Settings | File Templates.
 */
public class ProviderAddCommand implements Command
{
    protected String providerId = null;

    public void process(CommandContext ctx)
    {
        String out = "Provider '"+providerId+"'";
        try {
            ProviderManager manager = ctx.getProviderManager();
            Provider provider = manager.getProviderByName(providerId);
            if (provider != null) throw new Exception("Provider already exists");
            manager.createProvider(providerId);
            ctx.setMessage(out+" added");
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (Exception e) {
            ctx.setMessage("Couldn't add "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public void setProviderId(String providerId) {
        this.providerId = providerId;
    }

    public String getId() {
        return "PROVIDER_ADD";
    }
}

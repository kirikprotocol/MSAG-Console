package ru.novosoft.smsc.admin.console.commands.misc;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.admin.provider.ProviderManager;

import java.util.Iterator;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 12.04.2005
 * Time: 14:48:17
 * To change this template use File | Settings | File Templates.
 */
public class ProviderListCommand extends CommandClass
{
    public void process(CommandContext ctx)
    {
        try {
            ProviderManager manager = ctx.getProviderManager();
            Iterator i = manager.getProviders().keySet().iterator();
            if (!i.hasNext()) {
                ctx.setMessage("No providers defined");
                ctx.setStatus(CommandContext.CMD_OK);
            } else {
                while (i.hasNext()) {
                    Long providerId = (Long)i.next();
                    Provider provider = manager.getProvider(providerId);
                    if (provider != null) {
                        String providerName = (provider.getName() != null) ? provider.getName():"";
                        ctx.addResult(""+providerId.longValue()+" '"+providerName+"'");
                    }
                }
                ctx.setMessage("Providers list");
                ctx.setStatus(CommandContext.CMD_LIST);
            }
        } catch (Exception e) {
            e.printStackTrace();
            ctx.setMessage("Couldn't list providers. Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "PROVIDER_LIST";
    }
}

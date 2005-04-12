package ru.novosoft.smsc.admin.console.commands.misc;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.admin.provider.ProviderManager;

import java.util.Iterator;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 12.04.2005
 * Time: 14:48:17
 * To change this template use File | Settings | File Templates.
 */
public class ProviderListCommand  implements Command
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
                    String providerId = (String)i.next();
                    if (providerId != null && providerId.length() > 0)
                        ctx.addResult(providerId);
                }
                ctx.setMessage("Providers list");
                ctx.setStatus(CommandContext.CMD_LIST);
            }
        } catch (Exception e) {
            ctx.setMessage("Couldn't list providers. Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "PROVIDER_LIST";
    }
}

package ru.novosoft.smsc.admin.console.commands.apply;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 26.05.2005
 * Time: 16:18:45
 * To change this template use File | Settings | File Templates.
 */
public class ApplyProvidersCommand implements Command
{
    public void process(CommandContext ctx)
    {
        try {
            ctx.getProviderManager().apply();
            ctx.setMessage("Proviers applied succesfully");
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (Exception e) {
            ctx.setMessage("Couldn't apply provider(s) changes. Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            e.printStackTrace();
        }
    }

    public String getId() {
        return "ROVIDERS_APPLY";
    }
}

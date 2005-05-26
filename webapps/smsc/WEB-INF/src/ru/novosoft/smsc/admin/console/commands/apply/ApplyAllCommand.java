/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:24:53 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.apply;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;

public class ApplyAllCommand implements Command
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
}


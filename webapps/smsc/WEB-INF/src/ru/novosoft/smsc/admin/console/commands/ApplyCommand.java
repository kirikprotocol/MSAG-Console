/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:24:53 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;

public class ApplyCommand implements Command
{
    public void process(CommandContext ctx)
    {
        try {
            //ctx.getSmsc().applyProfiles();
            ctx.getSmsc().applyAliases();
            ctx.getSmsc().applyRoutes();
            ctx.setMessage("Changes applied succesfully");
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (Exception e) {
            ctx.setMessage("Failed to apply changes");
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            e.printStackTrace();
        }
    }
}


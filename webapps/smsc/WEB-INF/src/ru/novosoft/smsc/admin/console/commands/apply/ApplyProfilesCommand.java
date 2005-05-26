package ru.novosoft.smsc.admin.console.commands.apply;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.Command;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 26.05.2005
 * Time: 16:06:59
 * To change this template use File | Settings | File Templates.
 */
public class ApplyProfilesCommand implements Command
{
    public void process(CommandContext ctx)
    {
        try {
            ctx.getSmsc().applyProfiles();
            ctx.setMessage("Profiles applied succesfully");
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (Exception e) {
            ctx.setMessage("Couldn't apply profile(s) changes. Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            e.printStackTrace();
        }
    }

    public String getId() {
        return "PROFILES_APPLY";
    }
}


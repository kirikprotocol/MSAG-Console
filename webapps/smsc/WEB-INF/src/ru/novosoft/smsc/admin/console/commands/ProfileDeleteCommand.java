/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:58:28 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;

import ru.novosoft.smsc.admin.route.Mask;

public class ProfileDeleteCommand implements Command
{
    private String mask;

    public void setMask(String mask) {
        this.mask = mask;
    }

    public void process(CommandContext ctx)
    {
        ctx.setMessage("Depricated !!!");
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);

        /*String out = "Profile for mask '"+mask+"' ";
        try {
            ctx.getSmsc().updateProfile(new Mask(mask), null);
            ctx.setMessage(out+"deleted");
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (Exception e) {
            ctx.setMessage(out+"not deleted. Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }*/
    }
}


/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:54:05 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.commands.exceptions.CommandProcessException;
import ru.novosoft.smsc.admin.console.commands.exceptions.CommandParseException;
import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.route.Mask;

import java.util.Hashtable;

public class AliasAlterCommand implements Command
{
    private String address = null;
    private String alias = null;
    private boolean hide = false;

    public void setAddress(String address) {
        this.address = address;
    }
    public void setAlias(String alias) {
        this.alias = alias;
    }
    public void setHide(boolean hide) {
        this.hide = hide;
    }

    public void process(CommandContext ctx)
    {
        String out = "Alias '"+alias+"'";
        boolean ok = ctx.getSmsc().getAliases().remove(alias);
        if (!ok) {
            ctx.setMessage(out+" not exists");
            ctx.setResult(CommandContext.CMD_PROCESS_ERROR);
            return;
        }
        try {
            Alias smscAlias = new Alias(new Mask(address), new Mask(alias), hide);
            ctx.getSmsc().getAliases().add(smscAlias);
        } catch (Exception e) {
            e.printStackTrace();
            ctx.setMessage("Failed to update "+out+". Cause: "+e.getMessage());
            ctx.setResult(CommandContext.CMD_PROCESS_ERROR);
            return;
        }

        ctx.setMessage(out+" updated");
        ctx.setResult(CommandContext.CMD_OK);
    }
}


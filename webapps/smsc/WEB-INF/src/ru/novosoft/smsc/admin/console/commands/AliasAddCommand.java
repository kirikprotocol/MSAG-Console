/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:47:18 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;

import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.route.Mask;

public class AliasAddCommand implements Command
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
        String out = "Alias '"+alias+"' for address '"+address+"'";
        boolean ok = false;
        try {
            Alias smscAlias = new Alias(new Mask(address), new Mask(alias), hide);
            ok = ctx.getSmsc().getAliases().add(smscAlias);
            if (ok) {
                ctx.setMessage(out+" added");
                ctx.setResult(CommandContext.CMD_OK);
            } else {
                ctx.setMessage(out+" already exists");
                ctx.setResult(CommandContext.CMD_PROCESS_ERROR);
            }
        } catch (Exception e) {
            e.printStackTrace();
            ctx.setMessage("Failed to add "+out+". Cause: "+e.getMessage());
            ctx.setResult(CommandContext.CMD_PROCESS_ERROR);
        }
    }
}

/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:53:05 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.alias.Alias;

import java.util.Iterator;

public class AliasViewCommand implements Command
{
    private String alias;

    public void setAlias(String alias) {
        this.alias = alias;
    }

    private String showAlias(Alias alias)
    {
        return "Alias '"+alias.getAlias().getMask()+"'"+
               " = Address '"+alias.getAddress().getMask()+"'"+
               " ("+((alias.isHide()) ? "hide":"nohide")+")";
    }
    public void process(CommandContext ctx)
    {
        Alias smscAlias = ctx.getSmsc().getAliases().get(alias);
        if (smscAlias != null) {
            ctx.setMessage(showAlias(smscAlias));
            ctx.setStatus(CommandContext.CMD_OK);
        }
        else {
            ctx.setMessage("Alias '"+alias+"' not found");
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }
}


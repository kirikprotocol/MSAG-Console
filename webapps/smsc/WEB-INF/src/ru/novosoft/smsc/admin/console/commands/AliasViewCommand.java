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
               " ("+((alias.isHide()) ? "hidden":"public")+")";
    }
    public void process(CommandContext ctx)
    {
        String out;
        if (alias == null || alias.length() == 0) {
            Iterator i = ctx.getSmsc().getAliases().iterator();
            out = (i.hasNext()) ? "Aliases:" : "No aliases defined";
            while (i.hasNext()) {
                Alias smscAlias = (Alias)i.next();
                if (smscAlias != null) {
                    out += "\r\n" + showAlias(smscAlias);
                }
            }
        }
        else {
            Alias smscAlias = ctx.getSmsc().getAliases().get(alias);
            out = (smscAlias != null) ?
                    showAlias(smscAlias) : "Alias '"+alias+"' not found";
        }

        ctx.setMessage(out);
    }

}


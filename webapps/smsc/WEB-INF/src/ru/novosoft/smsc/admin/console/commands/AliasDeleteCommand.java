/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:49:04 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;

public class AliasDeleteCommand implements Command
{
    private String alias = null;

    public void setAlias(String alias) {
        this.alias = alias;
    }

    public void process(CommandContext ctx)
    {
        boolean ok = ctx.getSmsc().getAliases().remove(alias);
        ctx.setMessage((ok) ? "Alias '"+alias+"' deleted" : "Alias '"+alias+"' not found");
        ctx.setResult((ok) ? 0:-1);
    }
}


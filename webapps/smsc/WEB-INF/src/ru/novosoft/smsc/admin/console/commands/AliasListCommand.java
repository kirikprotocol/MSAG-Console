/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 19, 2002
 * Time: 2:35:30 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.alias.Alias;

import java.util.Iterator;

public class AliasListCommand implements Command
{
    public void process(CommandContext ctx)
    {
        try
        {
            Iterator i = ctx.getSmsc().getAliases().iterator();
            if (!i.hasNext()) {
                ctx.setMessage("No aliases defined");
                ctx.setStatus(CommandContext.CMD_OK);
            }
            else {
                while (i.hasNext()) {
                    Alias alias = (Alias)i.next();
                    if (alias != null) {
                        ctx.addResult(alias.getAlias().getMask());
                    }
                }
                ctx.setMessage("Alias list");
                ctx.setStatus(CommandContext.CMD_LIST);
            }
        }
        catch (Exception e) {
            ctx.setMessage("Couldn't list aliases. Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            return;
        }
    }

    public String getId() {
        return "ALIAS_LIST";
    }

}

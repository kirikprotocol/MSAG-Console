/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Jan 17, 2003
 * Time: 7:37:52 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.route.Mask;

import java.util.Iterator;

public class AliasShowCommand implements Command
{
    private String address = null;

    public void setAddress(String address) {
        this.address = address;
    }

    public void process(CommandContext ctx)
    {
        String out = "for address '"+address+"'";
        try
        {
            Mask addrMask = new Mask(address);
            Iterator i = ctx.getSmsc().getAliases().iterator();
            if (!i.hasNext()) {
                ctx.setMessage("No aliases defined at all");
                ctx.setStatus(CommandContext.CMD_OK);
                return;
            }
            while (i.hasNext()) {
                Alias alias = (Alias)i.next();
                String mask = alias.getAddress().getMask();
                if (mask.equals(addrMask.getMask())) {
                    ctx.addResult(alias.getAlias().getMask());
                }
            }
            if (ctx.getResults().size() > 0) {
                ctx.setStatus(CommandContext.CMD_LIST);
                ctx.setMessage("Alias list "+out);
            } else {
                ctx.setStatus(CommandContext.CMD_OK);
                ctx.setMessage("No aliases defined "+out);
            }
        }
        catch (Exception e) {
            ctx.setMessage("Couldn't list aliases "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            return;
        }
    }

    public String getId() {
        return "ALIAS_SHOW";
    }
}

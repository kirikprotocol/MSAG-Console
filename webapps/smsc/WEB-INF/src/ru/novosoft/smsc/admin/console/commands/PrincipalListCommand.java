/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Mar 7, 2003
 * Time: 4:57:57 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.dl.DistributionListAdmin;
import ru.novosoft.smsc.admin.dl.Principal;

import java.util.Iterator;

public class PrincipalListCommand implements Command
{
    public void process(CommandContext ctx)
    {
        try {
            DistributionListAdmin admin = ctx.getSmsc().getDistributionListAdmin();
            Iterator i = admin.principals().iterator();
            while (i.hasNext()) {
                Principal prc = (Principal)i.next();
                ctx.addResult(prc.getAddress());
            }
            ctx.setStatus(ctx.CMD_LIST);
        } catch (Exception e) {
            ctx.setMessage("Couldn't list principals. Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "PRINCIPAL_LIST";
    }
}

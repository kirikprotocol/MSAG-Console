/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Mar 11, 2003
 * Time: 12:23:58 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.dl.DistributionListAdmin;
import ru.novosoft.smsc.admin.dl.DistributionList;

import java.util.Iterator;


public class DistributionListListCommand implements Command
{
    public void process(CommandContext ctx)
    {
        try {
            DistributionListAdmin admin = ctx.getSmsc().getDistributionListAdmin();
            Iterator i = admin.list().iterator();
            while (i.hasNext()) {
                DistributionList list = (DistributionList)i.next();
                ctx.addResult(list.getName());
            }
            ctx.setMessage("Distribution lists");
            ctx.setStatus(CommandContext.CMD_LIST);
        } catch (Exception e) {
            ctx.setMessage("Couldn't list distribution lists. Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "DL_LIST";
    }
}

/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Mar 11, 2003
 * Time: 12:23:11 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.dl.DistributionListAdmin;
import ru.novosoft.smsc.admin.dl.DistributionList;
import ru.novosoft.smsc.admin.dl.exceptions.ListNotExistsException;

import java.util.Iterator;

public class DistributionListViewCommand implements Command
{
    private String name = null;

    public void setName(String name) {
        this.name = name;
    }

    public void process(CommandContext ctx)
    {
        String out = "Distribution list '"+name+"'";
        try {
            DistributionListAdmin admin = ctx.getSmsc().getDistributionListAdmin();
            DistributionList dl = admin.getDistributionList(name);
            if (dl == null)
                throw new ListNotExistsException(name);
            ctx.setMessage(out);
            ctx.addResult("Owner: "+(dl.isSys() ? "system":dl.getOwner()));
            ctx.addResult("Max elements: "+dl.getMaxElements());
            String memInfo = "";
            Iterator i = admin.members(name).iterator();
            while (i.hasNext()) {
                memInfo += (String)i.next();
                if (i.hasNext()) memInfo += ", ";
            }
            ctx.addResult("Members: "+memInfo);
            String subInfo = "";
            i = admin.submitters(name).iterator();
            while (i.hasNext()) {
                subInfo += (String)i.next();
                if (i.hasNext()) subInfo += ", ";
            }
            ctx.addResult("Submitters: "+subInfo);
            ctx.setStatus(ctx.CMD_LIST);
        } catch (ListNotExistsException e) {
            ctx.setMessage(out+" not exists");
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        } catch (Exception e) {
            ctx.setMessage("Couldn't view "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "DL_VIEW";
    }
}

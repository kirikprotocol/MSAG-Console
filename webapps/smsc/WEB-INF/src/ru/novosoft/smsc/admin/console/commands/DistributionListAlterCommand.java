/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Mar 11, 2003
 * Time: 12:21:20 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.dl.DistributionListAdmin;
import ru.novosoft.smsc.admin.dl.exceptions.ListNotExistsException;

public class DistributionListAlterCommand implements Command
{
    private String name = null;
    private int maxElements = 0;

    public void setName(String name) {
        this.name = name;
    }
    public void setMaxElements(int maxElements) {
        this.maxElements = maxElements;
    }

    public void process(CommandContext ctx)
    {
        String out = "Distribution list '"+name+"'";
        try {
            DistributionListAdmin admin = ctx.getSmsc().getDistributionListAdmin();
            admin.alterDistributionList(name, maxElements);
            ctx.setMessage(out+" altered");
            ctx.setStatus(ctx.CMD_OK);
        } catch (ListNotExistsException e) {
            ctx.setMessage(out+" not exists");
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        } catch (Exception e) {
            ctx.setMessage("Couldn't alter "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "DL_ALTER";
    }
}
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
        // todo implement maxEl alter in DistributionListAdmin
        DistributionListAdmin admin = ctx.getSmsc().getDistributionListAdmin();
        ctx.setMessage("Not implemented yet");
        ctx.setStatus(ctx.CMD_PROCESS_ERROR);
    }

    public String getId() {
        return "DL_ALTER";
    }
}
/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Mar 11, 2003
 * Time: 12:18:52 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;

public class DistributionListAddCommand implements Command
{
    private String name = null;
    private String owner = null;
    private int maxElements = 0;

    public void setName(String name) {
        this.name = name;
    }
    public void setOwner(String owner) {
        this.owner = owner;
    }
    public void setMaxElements(int maxElements) {
        this.maxElements = maxElements;
    }

    public void process(CommandContext ctx)
    {
        ctx.setMessage("Not implemented yet");
        ctx.setStatus(ctx.CMD_PROCESS_ERROR);
    }

    public String getId() {
        return "DL_ADD";
    }
}

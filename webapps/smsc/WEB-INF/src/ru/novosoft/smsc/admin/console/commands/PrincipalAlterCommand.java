/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Mar 7, 2003
 * Time: 4:37:35 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;

public class PrincipalAlterCommand implements Command
{
    private String address = null;
    private int maxLists = 0;
    private int maxElements = 0;

    private boolean setMaxLists = false;
    private boolean setMaxElements = false;

    public void setAddress(String address) {
        this.address = address;
    }
    public void setMaxLists(int maxLists) {
        this.maxLists = maxLists; setMaxLists = true;
    }
    public void setMaxElements(int maxElements) {
        this.maxElements = maxElements; setMaxElements = true;
    }

    public void process(CommandContext ctx)
    {
        ctx.setMessage("Not implemented yet");
        ctx.setStatus(ctx.CMD_PROCESS_ERROR);
    }

    public String getId() {
        return "PRINCIPAL_ALTER";
    }
}

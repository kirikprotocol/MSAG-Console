/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Mar 7, 2003
 * Time: 4:54:39 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;

public class PrincipalDeleteCommand implements Command
{
    private String address = null;

    public void setAddress(String address) {
        this.address = address;
    }

    public void process(CommandContext ctx)
    {
        ctx.setMessage("Not implemented yet");
        ctx.setStatus(ctx.CMD_PROCESS_ERROR);
    }

    public String getId() {
        return "PRINCIPAL_DELETE";
    }
}

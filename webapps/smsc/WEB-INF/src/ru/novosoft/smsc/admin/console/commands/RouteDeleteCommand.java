/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:09:06 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;

public class RouteDeleteCommand implements Command
{
    private String route;

    public void setRoute(String route) {
        this.route = route;
    }

    public void process(CommandContext ctx) {
        ctx.setMessage("Not implemented yet");
    }
}


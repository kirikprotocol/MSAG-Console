/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 19, 2002
 * Time: 6:07:20 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;

public class RouteViewCommand implements Command
{
    private String route;

    public void setRoute(String route) {
        this.route = route;
    }

    public void process(CommandContext ctx) {
        ctx.setMessage("Not implemented yet");
    }
}

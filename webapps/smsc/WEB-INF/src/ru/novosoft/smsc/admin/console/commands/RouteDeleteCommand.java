/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:09:06 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


public class RouteDeleteCommand implements Command
{
    private final static String OPTION_ROUTE = "route";

    public String process(String cmd) {
        return "RouteDeleteCommand";
    }
}


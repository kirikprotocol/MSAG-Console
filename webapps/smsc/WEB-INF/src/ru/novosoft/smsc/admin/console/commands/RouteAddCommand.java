/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:06:42 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


public class RouteAddCommand implements Command
{
    private final static String OPTION_ROUTE = "route";
    private final static String OPTION_BR = "br";
    private final static String OPTION_AR = "ar";
    private final static String OPTION_ALLOW = "allow";
    private final static String OPTION_SERVICE = "service";

    public String process(String cmd) {
        return "RouteAddCommand";
    }
}


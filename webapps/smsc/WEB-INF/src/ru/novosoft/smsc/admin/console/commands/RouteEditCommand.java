/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:10:13 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.SmscCommand;
import ru.novosoft.smsc.admin.console.commands.exceptions.CommandProcessException;
import ru.novosoft.smsc.admin.smsc_service.Smsc;

public class RouteEditCommand extends SmscCommand
{
    private final static String OPTION_ROUTE = "route";
    private final static String OPTION_BR = "br";
    private final static String OPTION_AR = "ar";
    private final static String OPTION_ALLOW = "allow";
    private final static String OPTION_SERVICE = "service";
    private final static String OPTION_SRC = "src";
    private final static String OPTION_ADDSRC = "addsrc";
    private final static String OPTION_DST = "dst";
    private final static String OPTION_ADDDST = "adddst";
    private final static String OPTION_SUBS = "subs";
    private final static String OPTION_MASKS = "masks";
    private final static String OPTION_SME = "sme";

    public RouteEditCommand(Smsc smsc) {
        super(smsc);
    }

    public String process(String cmd) throws CommandProcessException {
        return "RouteEditCommand";
    }
}


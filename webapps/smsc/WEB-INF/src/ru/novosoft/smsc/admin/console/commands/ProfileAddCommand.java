/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:57:16 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.SmscCommand;
import ru.novosoft.smsc.admin.console.commands.exceptions.CommandProcessException;
import ru.novosoft.smsc.admin.smsc_service.Smsc;

public class ProfileAddCommand extends SmscCommand
{
    private final static String OPTION_MASK = "mask";
    private final static String OPTION_REP = "rep";
    private final static String OPTION_CP = "cp";

    public ProfileAddCommand(Smsc smsc) {
        super(smsc);
    }

    public String process(String cmd) throws CommandProcessException {
        return "ProfileAddCommand";
    }
}


/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:58:28 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.SmscCommand;
import ru.novosoft.smsc.admin.console.commands.exceptions.CommandProcessException;
import ru.novosoft.smsc.admin.smsc_service.Smsc;

public class ProfileDeleteCommand extends SmscCommand
{
    private final static String OPTION_MASK = "mask";

    public ProfileDeleteCommand(Smsc smsc) {
        super(smsc);
    }

    public String process(String cmd) throws CommandProcessException {
        return "ProfileDeleteCommand";
    }
}


/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:55:34 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.SmscCommand;
import ru.novosoft.smsc.admin.console.commands.exceptions.CommandProcessException;
import ru.novosoft.smsc.admin.smsc_service.Smsc;

public class ProfileEditCommand extends SmscCommand
{
    private final static String OPTION_ADDRESS = "address";
    private final static String OPTION_CP = "cp";
    private final static String OPTION_REP = "rep";

    public ProfileEditCommand(Smsc smsc) {
        super(smsc);
    }

    public String process(String cmd) throws CommandProcessException {
        return "ProfileEditCommand";
    }
}


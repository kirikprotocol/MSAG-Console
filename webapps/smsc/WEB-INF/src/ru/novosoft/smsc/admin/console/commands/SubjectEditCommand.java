/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:05:13 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.SmscCommand;
import ru.novosoft.smsc.admin.console.commands.exceptions.CommandProcessException;
import ru.novosoft.smsc.admin.smsc_service.Smsc;

public class SubjectEditCommand extends SmscCommand
{
    private final static String OPTION_SUBJECT = "subject";
    private final static String OPTION_ADDMASKS = "addmasks";
    private final static String OPTION_MASKS = "masks";

    public SubjectEditCommand(Smsc smsc) {
        super(smsc);
    }

    public String process(String cmd) throws CommandProcessException {
        return "SubjectEditCommand";
    }
}


/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 17, 2002
 * Time: 11:44:22 AM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.smsc_service.Smsc;

public abstract class SmscCommand extends CommandParser implements Command
{
    protected Smsc smsc;

    protected SmscCommand(Smsc smsc) {
        this.smsc = smsc;
    }
}

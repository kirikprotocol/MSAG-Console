/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:49:04 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.SmscCommand;
import ru.novosoft.smsc.admin.console.commands.exceptions.CommandProcessException;
import ru.novosoft.smsc.admin.console.commands.exceptions.CommandParseException;
import ru.novosoft.smsc.admin.smsc_service.Smsc;

import java.util.Hashtable;

public class AliasDeleteCommand extends SmscCommand
{
    private final static String OPTION_ALIAS = "alias";

    public AliasDeleteCommand(Smsc smsc) {
        super(smsc);
    }

    public String process(String cmd)
        throws CommandProcessException
    {
        Hashtable params;
        try { params = parse(cmd); }
        catch (CommandParseException e) {
            throw new CommandProcessException(e.getMessage());
        }
        String alias = (String)params.get(OPTION_ALIAS);
        if (alias == null || alias.length() == 0)
            throw new CommandProcessException("'alias' option missed");

        boolean ok = smsc.getAliases().remove(alias);
        return (ok) ? "Alias '"+alias+"' deleted" : "Alias '"+alias+"' not found";
    }
}


/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:53:05 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.SmscCommand;
import ru.novosoft.smsc.admin.console.commands.exceptions.CommandProcessException;
import ru.novosoft.smsc.admin.console.commands.exceptions.CommandParseException;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.alias.Alias;

import java.util.Hashtable;
import java.util.Iterator;

public class AliasViewCommand extends SmscCommand
{
    private final static String OPTION_ALIAS = "alias";

    public AliasViewCommand(Smsc smsc) {
        super(smsc);
    }

    private String showAlias(Alias alias)
    {
        return "Alias '"+alias.getAlias().getMask()+"'"+
               " = Address '"+alias.getAddress().getMask()+"'"+
               " ("+((alias.isHide()) ? "hiden":"public")+")";
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
        String out;
        if (alias == null || alias.length() == 0) {
            Iterator i = smsc.getAliases().iterator();
            out = (i.hasNext()) ? "Aliases:" : "No aliases defined";
            while (i.hasNext()) {
                Alias smscAlias = (Alias)i.next();
                if (smscAlias != null) {
                    out += "\r\n" + showAlias(smscAlias);
                }
            }
        }
        else {
            Alias smscAlias = smsc.getAliases().get(alias);
            out = (smscAlias != null) ?
                    showAlias(smscAlias) : "Alias '"+alias+"' not found";
        }

        return out;
    }

}


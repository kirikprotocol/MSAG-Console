/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:54:05 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.SmscCommand;
import ru.novosoft.smsc.admin.console.commands.exceptions.CommandProcessException;
import ru.novosoft.smsc.admin.console.commands.exceptions.CommandParseException;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.route.Mask;

import java.util.Hashtable;

public class AliasEditCommand extends SmscCommand
{
    private final static String OPTION_ADDRESS = "address";
    private final static String OPTION_ALIAS = "alias";
    private final static String OPTION_HIDE = "hide";

    public AliasEditCommand(Smsc smsc) {
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
        String address = (String)params.get(OPTION_ADDRESS);
        if (address == null || address.length() == 0)
            throw new CommandProcessException("'address' option missed");
        String hiden = (String)params.get(OPTION_HIDE);
        if (hiden != null && !hiden.equalsIgnoreCase("true") &&
                !hiden.equalsIgnoreCase("false"))
            throw new CommandProcessException("'hide' option value should be 'true' or 'false'");
        boolean hide = (hiden != null) && hiden.equalsIgnoreCase("true");

        String out = "Alias '"+alias+"'";
        boolean ok = smsc.getAliases().remove(alias);
        if (!ok)
            throw new CommandProcessException(out+" not exists");
        try {
            Alias smscAlias = new Alias(new Mask(address), new Mask(alias), hide);
            smsc.getAliases().add(smscAlias);
        } catch (Exception e) {
            e.printStackTrace();
            throw new CommandProcessException(
                    "Failed to update "+out+". Cause: "+e.getMessage(), e);
        }

        return out+" updated";
    }
}


/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 17, 2002
 * Time: 2:52:54 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console;

import ru.novosoft.smsc.admin.console.commands.exceptions.CommandParseException;

import java.util.Hashtable;

public class CommandParser
{
    protected int firstDelimeter(String str)
    {
        int index = str.indexOf(' ');
        int rindex = str.indexOf('\r');
        if (rindex>0 && rindex<index) index = rindex;
        int tindex = str.indexOf('\t');
        if (tindex>0 && tindex<index) index = tindex;
        return (index <= 0) ? str.length():index;
    }

    public Hashtable parse(String cmd)
        throws CommandParseException
    {
        Hashtable hash = new Hashtable();
        String input;
        if (cmd == null || (input = cmd.trim()).length() == 0)
            return hash;

        while (input.length() > 0) {
            int eidx = input.indexOf('=');
            if (eidx <= 0)
                throw new CommandParseException("Command option is invalid, '=' missed");
            String option = input.substring(0, eidx).trim();
            if (option == null || option.length() == 0)
                throw new CommandParseException("Command option name missed");
            input = input.substring(eidx+1).trim();
            int didx = firstDelimeter(input);
            String value = input.substring(0, didx).trim();
            if (value == null || value.length() == 0)
                throw new CommandParseException("Command option value missed");
            input = input.substring(didx).trim();
            hash.put(option.toLowerCase(), value);
        }
        return hash;
    }
}

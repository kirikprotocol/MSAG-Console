/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 4:59:36 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.commands.exceptions.CommandProcessException;
import ru.novosoft.smsc.admin.console.commands.exceptions.CommandRegisterException;
import ru.novosoft.smsc.admin.smsc_service.Smsc;

import java.util.Hashtable;

public class CommandGroup extends CommandParser implements Command
{
    private Hashtable commands = new Hashtable();

    public void register(String name, Command command)
        throws CommandRegisterException
    {
        if (commands.containsKey(name)) {
            throw new CommandRegisterException("Command '"+name+"'already registered");
        }
        commands.put(name, command);
    }

    public String process(String cmd)
        throws CommandProcessException
    {
        if (cmd == null || cmd.length() == 0)
            throw new CommandProcessException("Incorrect command");
        String input = cmd.trim();
        int index = firstDelimeter(input);
        if (index <= 0)
            throw new CommandProcessException("Incorrect command");

        String first = input.substring(0, index);
        String rest = input.substring(index);
        Command command = (Command)commands.get(first);
        if (command == null)
            throw new CommandProcessException("No such command ('"+first+"')");

        return command.process(rest);
    }

}

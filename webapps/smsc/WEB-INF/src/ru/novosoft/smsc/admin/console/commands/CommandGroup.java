/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 4:59:36 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.commands.Command;

import java.util.Hashtable;

public class CommandGroup implements Command
{
    private Hashtable commands = new Hashtable();

    public void register(String name, Command command) {
        commands.put(name, command);
    }

    public String process(String cmd) {
        if (cmd != null) {
            String input = cmd.trim();
            int index = delimeterPosition(input);
            if (index > 0) {
                String first = input.substring(0, index);
                String rest = input.substring(index);
                Command command = (Command)commands.get(first);
                if (command != null) {
                    return command.process(rest);
                }
                return "Command '"+first+"' is not supported";
            }
        }
        return "Command is incorrect";
    }

    private int delimeterPosition(String str)
    {
        int index = str.indexOf(' ');
        int rindex = str.indexOf('\r');
        if (rindex>0 && rindex<index) index = rindex;
        int tindex = str.indexOf('\t');
        if (tindex>0 && tindex<index) index = tindex;
        return (index <= 0) ? str.length():index;
    }
}

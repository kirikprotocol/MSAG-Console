/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:58:28 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;

public class ProfileViewCommand implements Command
{
    private String address;

    public void setAddress(String address) {
        this.address = address;
    }

    public void process(CommandContext ctx) {
        ctx.setMessage("Not implemented yet");
    }
}


/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:55:34 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;

import ru.novosoft.smsc.admin.route.Mask;

public class ProfileAlterCommand implements Command
{
    private String address;
    private String report;
    private String enc;

    public void setAddress(String address) {
        this.address = address;
    }
    public void setReport(String rep) {
        this.report = rep;
    }
    public void setEncoding(String enc) {
        this.enc = enc;
    }

    public void process(CommandContext ctx)
    {
        ctx.setMessage("Not implemented yet");
    }
}


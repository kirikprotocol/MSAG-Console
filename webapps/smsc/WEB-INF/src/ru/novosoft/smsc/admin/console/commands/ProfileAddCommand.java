/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:57:16 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;

public class ProfileAddCommand implements Command
{
    private String mask;
    private String report;
    private String enc;

    public void setMask(String mask) {
        this.mask = mask;
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


/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Mar 11, 2003
 * Time: 1:07:40 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;

public class MemberAddCommand implements Command
{
    private String name = null;
    private String member = null;

    public void setName(String name) {
        this.name = name;
    }
    public void setMember(String member) {
        this.member = member;
    }

    public void process(CommandContext ctx)
    {
        ctx.setMessage("Not implemented yet");
        ctx.setStatus(ctx.CMD_PROCESS_ERROR);
    }

    public String getId() {
        return "MEMBER_ADD";
    }
}
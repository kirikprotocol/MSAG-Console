/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Mar 11, 2003
 * Time: 1:12:59 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.dl.DistributionListAdmin;
import ru.novosoft.smsc.admin.dl.exceptions.ListNotExistsException;
import ru.novosoft.smsc.admin.dl.exceptions.MemberNotExistsException;

public class MemberDeleteCommand implements Command
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
        String mout = "Member '"+member+"'";
        String dlout = "Distribution list '"+name+"'";
        try {
            DistributionListAdmin admin = ctx.getSmsc().getDistributionListAdmin();
            admin.deleteMember(name, member);
            ctx.setMessage(mout+" "+" deleted from "+dlout);
            ctx.setStatus(ctx.CMD_OK);
        } catch (ListNotExistsException e) {
            ctx.setMessage(dlout+" not exists");
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        } catch (MemberNotExistsException e) {
            ctx.setMessage(mout+" not registered in "+dlout);
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        } catch (Exception e) {
            ctx.setMessage("Couldn't delete "+mout+" from "+dlout+
                           ". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "MEMBER_DELETE";
    }
}
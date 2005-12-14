/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Mar 11, 2003
 * Time: 1:12:59 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.dl;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.dl.DistributionListAdmin;
import ru.novosoft.smsc.admin.dl.exceptions.ListNotExistsException;
import ru.novosoft.smsc.admin.dl.exceptions.MemberNotExistsException;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;

public class MemberDeleteCommand extends CommandClass
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
            ctx.setStatus(CommandContext.CMD_OK);
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

	public void updateJournalAndStatuses(CommandContext ctx, String userName)
	{
		journalAppend(ctx, userName, SubjectTypes.TYPE_dl, name, Actions.ACTION_MODIFY);
	}
}
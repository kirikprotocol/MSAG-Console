/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Mar 11, 2003
 * Time: 1:15:58 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.dl;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.dl.DistributionListAdmin;
import ru.novosoft.smsc.admin.dl.exceptions.ListNotExistsException;
import ru.novosoft.smsc.admin.dl.exceptions.SubmitterNotExistsException;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.journal.Actions;

public class SubmitterDeleteCommand extends CommandClass
{
    private String name = null;
    private String submitter = null;

    public void setName(String name) {
        this.name = name;
    }
    public void setSubmitter(String submitter) {
        this.submitter = submitter;
    }

    public void process(CommandContext ctx)
    {
        String sout = "Submitter '"+submitter+"'";
        String dlout = "Distribution list '"+name+"'";
        try {
            DistributionListAdmin admin = ctx.getSmsc().getDistributionListAdmin();
            admin.revokePosting(name, submitter);
            ctx.setMessage(sout+" "+" deleted from "+dlout);
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (ListNotExistsException e) {
            ctx.setMessage(dlout+" not exists");
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        } catch (SubmitterNotExistsException e) {
            ctx.setMessage(sout+" not registered in "+dlout);
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        } catch (Exception e) {
            ctx.setMessage("Couldn't add "+sout+" to "+dlout+
                           ". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "SUBMITTER_DELETE";
    }

	public void updateJournalAndStatuses(CommandContext ctx, String userName)
	{
		journalAppend(ctx, userName, SubjectTypes.TYPE_dl, name, Actions.ACTION_MODIFY);
	}
}

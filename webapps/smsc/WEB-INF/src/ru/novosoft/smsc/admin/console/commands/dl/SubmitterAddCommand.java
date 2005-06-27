/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Mar 11, 2003
 * Time: 1:13:35 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.dl;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.dl.DistributionListAdmin;
import ru.novosoft.smsc.admin.dl.exceptions.ListNotExistsException;
import ru.novosoft.smsc.admin.dl.exceptions.PrincipalNotExistsException;
import ru.novosoft.smsc.admin.dl.exceptions.SubmitterAlreadyExistsException;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.journal.Actions;

public class SubmitterAddCommand extends CommandClass
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
        String pout = "Principal for address '"+submitter+"'";
        String sout = "Submitter '"+submitter+"'";
        String dlout = "Distribution list '"+name+"'";
        try {
            DistributionListAdmin admin = ctx.getSmsc().getDistributionListAdmin();
            admin.grantPosting(name, submitter);
            ctx.setMessage(sout+" "+" added to "+dlout);
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (ListNotExistsException e) {
            ctx.setMessage(dlout+" not exists");
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        } catch (PrincipalNotExistsException e) {
            ctx.setMessage(pout+" not exists");
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        } catch (SubmitterAlreadyExistsException e) {
            ctx.setMessage(sout+" already registered in "+dlout);
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        } catch (Exception e) {
            ctx.setMessage("Couldn't add "+sout+" to "+dlout+
                           ". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "SUBMITTER_ADD";
    }

	public void updateJournalAndStatuses(CommandContext ctx, String userName)
	{
		journalAppend(ctx, userName, SubjectTypes.TYPE_dl, name, Actions.ACTION_MODIFY);
	}
}

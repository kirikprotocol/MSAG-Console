/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:59:24 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.route;


import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.route.Subject;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.journal.Actions;

public class SubjectAddCommand extends SubjectGenCommand
{
    public void process(CommandContext ctx)
    {
        String out = "Subject '"+subject+"'";
        try {
            SME sme = ctx.getSmeManager().get(defaultSmeId);
            if (sme != null) {
                Subject smscSubject = new Subject(subject, masks, sme, notes);
                ctx.getRouteSubjectManager().getSubjects().add(smscSubject);
                ctx.setMessage(out+" added");
                ctx.setStatus(CommandContext.CMD_OK);
            } else {
                ctx.setMessage("Couldn't add "+out+". No default SME for id '"+defaultSmeId+"'");
                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            }
        } catch (Exception e) {
            ctx.setMessage("Couldn't add "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "SUBJECT_ADD";
    }

	public void updateJournalAndStatuses(CommandContext ctx, String userName)
	{
		journalAppend(ctx, userName, SubjectTypes.TYPE_subject, subject, Actions.ACTION_ADD);
		ctx.getStatuses().setSubjectsChanged(true);
	}
}

/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:00:32 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.route.Subject;

public class SubjectDeleteCommand extends SubjectGenCommand
{
    public void process(CommandContext ctx)
    {
        String out = "Subject '"+subject+"'";
        try {
            if (ctx.getSmsc().getRoutes().isSubjectUsed(subject))
                throw new Exception("subject is used in existing dst or src definition(s)");
            Subject smscSubject = ctx.getSmsc().getSubjects().remove(subject);
            if (smscSubject == null) {
                ctx.setMessage(out+" not found");
                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            } else {
                ctx.setMessage(out+" deleted");
                ctx.setStatus(CommandContext.CMD_OK);
            }
        } catch (Exception e) {
            ctx.setMessage("Couldn't delete "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "SUBJECT_DELETE";
    }

}


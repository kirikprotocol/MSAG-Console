/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:01:25 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.route.Subject;

public class SubjectViewCommand extends SubjectGenCommand
{
    private String showSubject(Subject subj)
    {
        // todo Show masks here ?
        return "Subject '"+subj.getName()+"'"+
               " Default SME id:"+subj.getDefaultSme().getId();
    }
    public void process(CommandContext ctx)
    {
        Subject smscSubject = ctx.getSmsc().getSubjects().get(subject);
        if (smscSubject != null) {
            ctx.setMessage(showSubject(smscSubject));
            ctx.setStatus(CommandContext.CMD_OK);
        }
        else {
            ctx.setMessage("Subject '"+subject+"' not found");
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }
}


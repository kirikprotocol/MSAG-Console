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
import ru.novosoft.smsc.admin.route.Mask;

import java.util.Iterator;

public class SubjectViewCommand extends SubjectGenCommand
{
    private String showSubject(Subject subj)
    {
        String out = "Subject '"+subj.getName()+"'"+
               " DefSME: "+subj.getDefaultSme().getId();
        Iterator i = subj.getMasks().iterator();
        if (i.hasNext()) out += " Mask(s): ";
        while (i.hasNext()) {
            out += ((Mask)i.next()).getMask();
            if (i.hasNext()) out += ", ";
        }
        return out;
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


/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 19, 2002
 * Time: 6:04:47 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.route.Subject;

import java.util.Iterator;

public class SubjectListCommand implements Command
{
    public void process(CommandContext ctx)
    {
        try
        {
            Iterator i = ctx.getRouteSubjectManager().getSubjects().iterator();
            if (!i.hasNext()) {
                ctx.setMessage("No subjects defined");
                ctx.setStatus(CommandContext.CMD_OK);
            }
            else {
                while (i.hasNext()) {
                    Subject subj = (Subject)i.next();
                    if (subj != null) {
                        ctx.addResult(subj.getName());
                    }
                }
                ctx.setMessage("Subjects list");
                ctx.setStatus(CommandContext.CMD_LIST);
            }
        }
        catch (Exception e) {
            ctx.setMessage("Couldn't list subjects. Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            return;
        }
    }

    public String getId() {
        return "SUBJECT_LIST";
    }

}

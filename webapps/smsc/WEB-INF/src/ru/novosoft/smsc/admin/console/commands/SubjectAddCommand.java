/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:59:24 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.route.Subject;
import ru.novosoft.smsc.admin.route.SME;

public class SubjectAddCommand extends SubjectGenCommand
{
    public void process(CommandContext ctx)
    {
        String out = "Subject '"+subject+"' ";
        try {
            SME sme = ctx.getSmsc().getSmes().get(defaultSmeId);
            if (sme != null) {
                ctx.getSmsc().getSubjects().add(new Subject(subject, ((String[])(masks.toArray())), sme));
                ctx.setMessage(out+" added");
                ctx.setStatus(CommandContext.CMD_OK);
            } else {
                ctx.setMessage("Failed to add "+out+
                               ". No default SME for id '"+defaultSmeId+"'");
                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            }
        } catch (Exception e) {
            ctx.setMessage("Failed to add "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }
}

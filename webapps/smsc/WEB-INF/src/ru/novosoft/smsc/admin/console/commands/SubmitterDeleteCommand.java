/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Mar 11, 2003
 * Time: 1:15:58 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.dl.DistributionListAdmin;
import ru.novosoft.smsc.admin.dl.exceptions.ListNotExistsException;
import ru.novosoft.smsc.admin.dl.exceptions.SubmitterNotExistsException;

public class SubmitterDeleteCommand implements Command
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
            ctx.setStatus(ctx.CMD_OK);
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
}

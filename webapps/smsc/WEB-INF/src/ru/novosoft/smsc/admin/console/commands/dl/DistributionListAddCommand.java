/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Mar 11, 2003
 * Time: 12:18:52 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.dl;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.dl.DistributionListAdmin;
import ru.novosoft.smsc.admin.dl.DistributionList;
import ru.novosoft.smsc.admin.dl.exceptions.ListAlreadyExistsException;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.journal.Actions;

public class DistributionListAddCommand extends CommandClass
{
    private String name = null;
    private String owner = null;
    private int maxElements = 0;

    public void setName(String name) {
        this.name = name;
    }
    public void setOwner(String owner) {
        this.owner = owner;
    }
    public void setMaxElements(int maxElements) {
        this.maxElements = maxElements;
    }

    public void process(CommandContext ctx)
    {
        String out = "Distribution list '"+name+"'";
        try {
            DistributionListAdmin admin = ctx.getSmsc().getDistributionListAdmin();
            admin.addDistributionList(new DistributionList(name, owner, maxElements));
            ctx.setMessage(out+" added");
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (ListAlreadyExistsException e) {
            ctx.setMessage(out+" already exists");
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        } catch (Exception e) {
            ctx.setMessage("Couldn't add "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "DL_ADD";
    }

	public void updateJournalAndStatuses(CommandContext ctx, String userName)
	{
		journalAppend(ctx, userName, SubjectTypes.TYPE_dl, name, Actions.ACTION_ADD);
	}
}

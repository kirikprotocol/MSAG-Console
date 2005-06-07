/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:49:04 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.alias;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.journal.Actions;

public class AliasDeleteCommand extends CommandClass
{
    private String alias = null;

    public void setAlias(String alias) {
        this.alias = alias;
    }

    public void process(CommandContext ctx)
    {
        String out = "Alias '"+alias+"' ";
        boolean ok = ctx.getSmsc().getAliases().remove(alias);
        ctx.setMessage((ok) ? out+"deleted" : out+"not found");
        ctx.setStatus((ok) ? CommandContext.CMD_OK : CommandContext.CMD_PROCESS_ERROR);
    }

    public String getId() {
        return "ALIAS_DELETE";
    }

	public void updateJournalAndStatuses(CommandContext ctx, String userName)
	{
		journalAppend(ctx, userName, SubjectTypes.TYPE_alias, alias, Actions.ACTION_DEL);
		ctx.getStatuses().setAliasesChanged(true);
	}
}


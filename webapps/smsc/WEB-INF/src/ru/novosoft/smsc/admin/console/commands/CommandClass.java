package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;

/**
 * Created by IntelliJ IDEA.
 * User: starkom
 * Date: 27.05.2005
 * Time: 15:12:21
 * To change this template use File | Settings | File Templates.
 */
public abstract class CommandClass implements Command
{
	public abstract void process(CommandContext ctx);
	public abstract String getId();

	public void updateJournalAndStatuses(CommandContext ctx, String userName)
	{
	}

	protected void journalAppend(CommandContext ctx, String userName, final byte subjectType, final String subjectId, final byte action)
	{
		ctx.getJournal().append(userName, Constants.CONSOLE_SESSION_ID, subjectType, subjectId, action);
	}

	protected void journalAppend(CommandContext ctx, String userName, final byte subjectType, final String subjectId, final byte action, final String additionalKey, final String additionalValue)
	{
		ctx.getJournal().append(userName, Constants.CONSOLE_SESSION_ID, subjectType, subjectId, action, additionalKey, additionalValue);
	}
}

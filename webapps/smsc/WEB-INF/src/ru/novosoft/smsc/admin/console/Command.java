/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 4:59:46 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console;

public interface Command
{
	public String getId();
	public void process(CommandContext ctx);
	public void updateJournalAndStatuses(CommandContext ctx, String userName);
}

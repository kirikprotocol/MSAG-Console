/*
 * Created by igork
 * Date: Mar 19, 2002
 * Time: 11:09:05 PM
 */
package ru.novosoft.smsc.admin.protocol;


public class CommandKillService extends ServiceCommand
{
	public CommandKillService(String serviceName)
	{
		super("kill_service", serviceName);
	}
}

/*
 * Created by igork
 * Date: Apr 2, 2002
 * Time: 9:24:19 PM
 */
package ru.novosoft.smsc.admin.protocol;


public class CommandListComponents extends ServiceCommand
{
	public CommandListComponents(String serviceId)
	{
		super("list_components", serviceId);
	}
}

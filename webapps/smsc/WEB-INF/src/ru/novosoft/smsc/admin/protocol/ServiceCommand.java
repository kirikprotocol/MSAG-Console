/*
 * Created by igork
 * Date: Apr 3, 2002
 * Time: 1:53:39 PM
 */
package ru.novosoft.smsc.admin.protocol;

import ru.novosoft.smsc.util.StringEncoderDecoder;


public class ServiceCommand extends Command
{
	public ServiceCommand(String commandName, String serviceId)
	{
		super(commandName);

		document.getDocumentElement().setAttribute("service", StringEncoderDecoder.encode(serviceId));
	}
}

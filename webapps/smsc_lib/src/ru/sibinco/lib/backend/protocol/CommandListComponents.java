package ru.sibinco.lib.backend.protocol;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.service.ServiceCommand;
import ru.sibinco.lib.backend.service.ServiceCommand;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.03.2005
 * Time: 13:29:25
 * To change this template use File | Settings | File Templates.
 */
public class CommandListComponents extends ServiceCommand
{
	public CommandListComponents(String serviceId) throws SibincoException
  {
		super("list_components", serviceId);
	}
}

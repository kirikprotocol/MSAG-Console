/*
 * Created by igork
 * Date: Mar 19, 2002
 * Time: 11:09:44 PM
 */
package ru.sibinco.lib.backend.daemon;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.service.ServiceCommand;


public class CommandRemoveService extends ServiceCommand
{
  public CommandRemoveService(String serviceName) throws SibincoException
  {
    super("remove_service", serviceName);
  }
}

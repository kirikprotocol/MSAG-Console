/*
 * Created by igork
 * Date: Mar 19, 2002
 * Time: 11:08:45 PM
 */
package ru.sibinco.lib.backend.daemon;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.service.ServiceCommand;


public class CommandShutdownService extends ServiceCommand
{
  public CommandShutdownService(String serviceName) throws SibincoException
  {
    super("shutdown_service", serviceName);
  }
}

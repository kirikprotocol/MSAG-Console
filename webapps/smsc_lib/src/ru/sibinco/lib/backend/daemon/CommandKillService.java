/*
 * Created by igork
 * Date: Mar 19, 2002
 * Time: 11:09:05 PM
 */
package ru.sibinco.lib.backend.daemon;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.service.ServiceCommand;


public class CommandKillService extends ServiceCommand
{
  public CommandKillService(String serviceName) throws SibincoException
  {
    super("kill_service", serviceName);
  }
}

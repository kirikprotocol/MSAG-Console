/*
 * Created by igork
 * Date: Mar 19, 2002
 * Time: 11:08:28 PM
 */
package ru.sibinco.lib.backend.daemon;

import ru.sibinco.lib.SibincoException;


public class CommandStartService extends ServiceCommand
{
  public CommandStartService(String serviceName) throws SibincoException
  {
    super("start_service", serviceName);
  }
}

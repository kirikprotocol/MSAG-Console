/*
 * Created by igork
 * Date: Mar 19, 2002
 * Time: 11:09:44 PM
 */
package ru.novosoft.smsc.admin.protocol;


public class CommandRemoveService extends ServiceCommand
{
  public CommandRemoveService(String serviceName)
  {
    super("remove_service", serviceName);
  }
}

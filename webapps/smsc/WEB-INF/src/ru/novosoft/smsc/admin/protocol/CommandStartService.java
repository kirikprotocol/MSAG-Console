/*
 * Created by igork
 * Date: Mar 19, 2002
 * Time: 11:08:28 PM
 */
package ru.novosoft.smsc.admin.protocol;


public class CommandStartService extends ServiceCommand
{
  public CommandStartService(String serviceName)
  {
    super("start_service", serviceName);
  }
}

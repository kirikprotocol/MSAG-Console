/*
 * Created by igork
 * Date: Mar 19, 2002
 * Time: 11:08:45 PM
 */
package ru.novosoft.smsc.admin.protocol;


public class CommandShutdownService extends ServiceCommand
{
  public CommandShutdownService(String serviceName)
  {
    super("shutdown_service", serviceName);
  }
}

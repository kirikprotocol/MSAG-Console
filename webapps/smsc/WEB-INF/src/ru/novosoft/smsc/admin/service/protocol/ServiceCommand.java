/*
 * Created by igork
 * Date: Apr 3, 2002
 * Time: 1:53:39 PM
 */
package ru.novosoft.smsc.admin.protocol;

public class ServiceCommand extends Command
{
  public ServiceCommand(String commandName, String serviceName)
  {
    super(commandName);

    document.getDocumentElement().setAttribute("service", serviceName);
  }
}

/*
 * Created by igork
 * Date: Apr 2, 2002
 * Time: 9:24:19 PM
 */
package ru.novosoft.smsc.admin.protocol;


public class CommandListComponents extends ServiceCommand
{
  public CommandListComponents(String serviceName)
  {
    super("list_components", serviceName);
  }
}

/*
 * Created by igork
 * Date: Mar 19, 2002
 * Time: 11:09:05 PM
 */
package ru.novosoft.smsc.admin.protocol;

import org.w3c.dom.Element;

public class CommandKillService extends Command
{
  public CommandKillService(String serviceName)
  {
    super("kill_service");

    Element serviceElem = document.createElement("service");
    document.getDocumentElement().appendChild(serviceElem);

    Element name = document.createElement("name");
    name.appendChild(document.createTextNode(serviceName));
    serviceElem.appendChild(name);
  }
}

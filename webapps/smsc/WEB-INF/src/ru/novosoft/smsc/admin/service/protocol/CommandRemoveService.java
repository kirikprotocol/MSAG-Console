/*
 * Created by igork
 * Date: Mar 19, 2002
 * Time: 11:09:44 PM
 */
package ru.novosoft.smsc.admin.protocol;

import org.w3c.dom.Element;

public class CommandRemoveService extends Command
{
  public CommandRemoveService(String serviceName)
  {
    super("remove_service");

    Element serviceElem = document.createElement("service");
    document.getDocumentElement().appendChild(serviceElem);

    Element name = document.createElement("name");
    name.appendChild(document.createTextNode(serviceName));
    serviceElem.appendChild(name);
  }
}

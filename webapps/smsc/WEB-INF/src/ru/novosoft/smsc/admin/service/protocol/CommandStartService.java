/*
 * Created by igork
 * Date: Mar 19, 2002
 * Time: 11:08:28 PM
 */
package ru.novosoft.smsc.admin.protocol;

import org.w3c.dom.Element;

public class CommandStartService extends Command
{
  public CommandStartService(String serviceName)
  {
    super("start_service");

    Element serviceElem = document.createElement("service");
    document.getDocumentElement().appendChild(serviceElem);

    Element name = document.createElement("name");
    name.appendChild(document.createTextNode(serviceName));
    serviceElem.appendChild(name);
  }
}

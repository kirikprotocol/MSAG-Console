/*
 * Created by igork
 * Date: Mar 19, 2002
 * Time: 11:09:26 PM
 */
package ru.novosoft.smsc.admin.protocol;

import org.w3c.dom.Element;

import ru.novosoft.smsc.admin.service.ServiceInfo;

public class CommandAddService extends Command
{
  public CommandAddService(ServiceInfo serviceInfo)
  {
    super("add_service");

    logger.debug("Add service \"" + serviceInfo.getName() + "\" (" + serviceInfo.getHost() + ':'
                 + serviceInfo.getPort() + ")");

    Element serviceElem = document.createElement("service");
    serviceElem.setAttribute("name", serviceInfo.getName());
    serviceElem.setAttribute("cmd_line", serviceInfo.getCmdLine());
    serviceElem.setAttribute("config_file", serviceInfo.getConfigFileName());
    serviceElem.setAttribute("port", String.valueOf(serviceInfo.getPort()));
    document.getDocumentElement().appendChild(serviceElem);

    for (int i = 0; i < serviceInfo.getArgs().size(); i++) {
      Element argElem = document.createElement("arg");
      argElem.setAttribute("num", Integer.toString(i));
      argElem.appendChild(document.createTextNode((String) serviceInfo.getArgs().elementAt(i)));
      serviceElem.appendChild(argElem);
      logger.debug("parameter[" + i + "]=\"" + (String) serviceInfo.getArgs().elementAt(i) + "\" added");
    }
    logger.debug("Command add_service created");
  }
}

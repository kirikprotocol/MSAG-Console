/*
 * Created by igork
 * Date: Mar 19, 2002
 * Time: 11:09:26 PM
 */
package ru.novosoft.smsc.admin.protocol;

import org.w3c.dom.Element;

import javax.xml.parsers.DocumentBuilderFactory;

import ru.novosoft.smsc.admin.service.ServiceInfo;

public class CommandAddService extends Command
{
  public CommandAddService(ServiceInfo serviceInfo)
  {
    super("add_service");

    logger.debug("Add service \"" + serviceInfo.getName() + "\" (" + serviceInfo.getHost() + ':'
                 + serviceInfo.getPort() + ")");

    Element serviceElem = document.createElement("service");
    document.getDocumentElement().appendChild(serviceElem);
    logger.debug("elem service created");

    Element name = document.createElement("name");
    name.appendChild(document.createTextNode(serviceInfo.getName()));
    serviceElem.appendChild(name);
    logger.debug("name \"" + serviceInfo.getName() + "\" added");

    Element cmdElem = document.createElement("cmd_line");
    cmdElem.appendChild(document.createTextNode(serviceInfo.getCmdLine()));
    serviceElem.appendChild(cmdElem);
    logger.debug("cmd_line \"" + serviceInfo.getCmdLine() + "\" added");

    Element portElem = document.createElement("port");
    portElem.appendChild(document.createTextNode(String.valueOf(serviceInfo.getPort())));
    serviceElem.appendChild(portElem);
    logger.debug("port \"" + serviceInfo.getPort() + "\" added");

    for (int i=0; i<serviceInfo.getArgs().length; i++)
    {
      logger.debug("adding parameter " + i);
      Element argElem = document.createElement("arg");
      argElem.setAttribute("num", Integer.toString(i));
      argElem.appendChild(document.createTextNode(serviceInfo.getArgs()[i]));
      serviceElem.appendChild(argElem);
      logger.debug("parameter[" + i + "]=\"" + serviceInfo.getArgs()[i] + "\" added");
    }
    logger.debug("Command add_service created");
  }
}

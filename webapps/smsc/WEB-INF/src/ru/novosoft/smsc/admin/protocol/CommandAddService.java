/*
 * Created by igork
 * Date: Mar 19, 2002
 * Time: 11:09:26 PM
 */
package ru.novosoft.smsc.admin.protocol;

import org.w3c.dom.Element;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.util.StringEncoderDecoder;


public class CommandAddService extends Command
{
  public CommandAddService(ServiceInfo serviceInfo)
  {
    super("add_service");

    logger.debug("Add service \"" + serviceInfo.getId() + "\" (" + serviceInfo.getHost() + ':'
                 + serviceInfo.getPort() + ")");

    Element serviceElem = document.createElement("service");
    //serviceElem.setAttribute("name", StringEncoderDecoder.encode(serviceInfo.getName()));
    serviceElem.setAttribute("id", StringEncoderDecoder.encode(serviceInfo.getId()));
    serviceElem.setAttribute("port", StringEncoderDecoder.encode(String.valueOf(serviceInfo.getPort())));
    serviceElem.setAttribute("args", StringEncoderDecoder.encode(String.valueOf(serviceInfo.getArgs())));
    document.getDocumentElement().appendChild(serviceElem);
    //logger.debug("Command add_service created");
  }
}

/*
 * Created by igork
 * Date: Mar 19, 2002
 * Time: 11:09:26 PM
 */
package ru.sibinco.lib.backend.daemon;

import org.w3c.dom.Element;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.protocol.Command;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.lib.backend.service.ServiceInfo;


public class CommandAddService extends Command
{
  public CommandAddService(final ServiceInfo serviceInfo) throws SibincoException
  {
    super("add_service");

    logger.debug("Add services \"" + serviceInfo.getId() + "\" (" + serviceInfo.getHost() + ")");

    final Element serviceElem = document.createElement("service");
    //serviceElem.setAttribute("name", StringEncoderDecoder.encode(serviceInfo.getName()));
    serviceElem.setAttribute("id", StringEncoderDecoder.encode(serviceInfo.getId()));
    serviceElem.setAttribute("args", StringEncoderDecoder.encode(serviceInfo.getArgs()));
    serviceElem.setAttribute("status", serviceInfo.getStatusStr());
    serviceElem.setAttribute("autostart", serviceInfo.isAutostart() ? "true" : "false");
    document.getDocumentElement().appendChild(serviceElem);
    //logger.debug("Command add_service created");
  }
}

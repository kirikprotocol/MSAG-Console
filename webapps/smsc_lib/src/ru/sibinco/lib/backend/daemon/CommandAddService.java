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


public class CommandAddService extends Command
{
  public CommandAddService(ServiceInfo serviceInfo) throws SibincoException
  {
    super("add_service");

    logger.debug("Add services \"" + serviceInfo.getId() + "\" (" + serviceInfo.getHost() + ':'
                 + serviceInfo.getPort() + ")");

    Element serviceElem = document.createElement("service");
    //serviceElem.setAttribute("name", StringEncoderDecoder.encode(serviceInfo.getName()));
    serviceElem.setAttribute("id", StringEncoderDecoder.encode(serviceInfo.getId()));
    serviceElem.setAttribute("port", StringEncoderDecoder.encode(String.valueOf(serviceInfo.getPort())));
    serviceElem.setAttribute("args", StringEncoderDecoder.encode(serviceInfo.getArgs()));
    serviceElem.setAttribute("status", serviceInfo.getStatusStr());
    serviceElem.setAttribute("autostart", serviceInfo.isAutostart() ? "true" : "false");
    document.getDocumentElement().appendChild(serviceElem);
    //logger.debug("Command add_service created");
  }
}

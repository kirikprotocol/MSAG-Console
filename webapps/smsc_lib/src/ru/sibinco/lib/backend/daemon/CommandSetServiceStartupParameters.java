/*
 * Author: igork
 * Date: 06.05.2002
 * Time: 16:10:22
 */
package ru.sibinco.lib.backend.daemon;

import org.w3c.dom.Element;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.lib.backend.service.ServiceCommand;


public class CommandSetServiceStartupParameters extends ServiceCommand
{
  public CommandSetServiceStartupParameters(String serviceId, /*String serviceName, */int port, String args) throws SibincoException
  {
    super("set_service_startup_parameters", serviceId);

    Element serviceElem = document.createElement("service");
    //serviceElem.setAttribute("name", StringEncoderDecoder.encode(serviceName));
    serviceElem.setAttribute("id", StringEncoderDecoder.encode(serviceId));
    serviceElem.setAttribute("port", StringEncoderDecoder.encode(String.valueOf(port)));
    serviceElem.setAttribute("args", StringEncoderDecoder.encode(String.valueOf(args)));
    document.getDocumentElement().appendChild(serviceElem);
  }
}

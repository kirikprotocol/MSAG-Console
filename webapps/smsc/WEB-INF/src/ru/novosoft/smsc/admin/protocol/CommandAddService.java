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
  public CommandAddService(final ServiceInfo serviceInfo)
  {
    super("add_service");

    logger.debug("Add services \"" + serviceInfo.getId() + "\" (" + serviceInfo.getHost() + ")");

    final Element serviceElem = document.createElement("service");
    serviceElem.setAttribute("id", StringEncoderDecoder.encode(serviceInfo.getId()));
    serviceElem.setAttribute("args", StringEncoderDecoder.encode(serviceInfo.getArgs()));
    serviceElem.setAttribute("status", serviceInfo.getStatusStr());
    serviceElem.setAttribute("autostart", serviceInfo.isAutostart() ? "true" : "false");
    document.getDocumentElement().appendChild(serviceElem);
  }
}

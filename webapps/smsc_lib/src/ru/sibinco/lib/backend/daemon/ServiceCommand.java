/*
 * Created by igork
 * Date: Apr 3, 2002
 * Time: 1:53:39 PM
 */
package ru.sibinco.lib.backend.daemon;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.protocol.Command;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;


public class ServiceCommand extends Command
{
  public ServiceCommand(String commandName, String serviceId) throws SibincoException
  {
    super(commandName);

    document.getDocumentElement().setAttribute("service", StringEncoderDecoder.encode(serviceId));
  }
}

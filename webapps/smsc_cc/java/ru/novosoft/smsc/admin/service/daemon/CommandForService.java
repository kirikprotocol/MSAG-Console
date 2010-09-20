package ru.novosoft.smsc.admin.service.daemon;

import ru.novosoft.smsc.admin.protocol.Command;
import ru.novosoft.smsc.util.StringEncoderDecoder;

/**
 * @author Artem Snopkov
 */
abstract class CommandForService extends Command {

  protected CommandForService(String commandName, String serviceId) {
    super(commandName);

    document.getDocumentElement().setAttribute("service", StringEncoderDecoder.encode(serviceId));
  }
}

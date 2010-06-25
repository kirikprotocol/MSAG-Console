package ru.novosoft.smsc.admin.callable;

import ru.novosoft.smsc.admin.protocol.Command;
import ru.novosoft.smsc.util.StringEncoderDecoder;


class ServiceCommand extends Command {
  public ServiceCommand(String commandName, String serviceId) {
    super(commandName);

    document.getDocumentElement().setAttribute("callable", StringEncoderDecoder.encode(serviceId));
  }
}

package mobi.eyeline.informer.admin.service.daemon;

import mobi.eyeline.informer.admin.protocol.Command;
import mobi.eyeline.informer.util.StringEncoderDecoder;
import org.w3c.dom.Element;

/**
 * Команда на добавление сервиса
 */
class CommandAddService extends Command {
  public CommandAddService(String id, String args, String status, boolean autostart) {
    super("add_service");

    final Element serviceElem = document.createElement("callable");
    serviceElem.setAttribute("id", StringEncoderDecoder.encode(id));
    serviceElem.setAttribute("args", StringEncoderDecoder.encode(args));
    serviceElem.setAttribute("status", status);
    serviceElem.setAttribute("autostart", autostart ? "true" : "false");
    document.getDocumentElement().appendChild(serviceElem);
  }
}

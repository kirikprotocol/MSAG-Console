package mobi.eyeline.informer.admin.service.daemon;

import mobi.eyeline.informer.admin.protocol.Command;
import mobi.eyeline.informer.util.StringEncoderDecoder;

/**
 * @author Artem Snopkov
 */
abstract class CommandForService extends Command {

  protected CommandForService(String commandName, String serviceId) {
    super(commandName);

    document.getDocumentElement().setAttribute("service", StringEncoderDecoder.encode(serviceId));
  }
}

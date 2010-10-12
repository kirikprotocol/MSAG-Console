package mobi.eyeline.informer.admin.service.daemon;

import mobi.eyeline.informer.admin.protocol.Command;

/**
 * Команда на получение списка сервисов
 */
class CommandListServices extends Command {

  public CommandListServices() {
    super("list_services");
  }
}

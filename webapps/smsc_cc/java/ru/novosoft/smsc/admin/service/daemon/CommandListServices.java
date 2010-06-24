package ru.novosoft.smsc.admin.service.daemon;

import ru.novosoft.smsc.admin.protocol.Command;

/**
 * Команда на получение списка сервисов
 */
class CommandListServices extends Command {

  public CommandListServices() {
    super("list_services");
  }
}

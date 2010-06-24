package ru.novosoft.smsc.admin.service.daemon;

/**
 * Команда на переключение сервиса с ноды на ноду
 */
class CommandSwitchOverService extends CommandForService {
  public CommandSwitchOverService(String serviceName) {
    super("switchover_service", serviceName);
  }
}

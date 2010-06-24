package ru.novosoft.smsc.admin.service.daemon;

/**
 * Команда на удаление сервиса
 */
class CommandRemoveService extends CommandForService {
  public CommandRemoveService(String serviceName) {
    super("remove_service", serviceName);
  }
}

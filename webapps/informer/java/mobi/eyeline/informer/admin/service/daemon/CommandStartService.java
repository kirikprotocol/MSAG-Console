package mobi.eyeline.informer.admin.service.daemon;

/**
 * Команда на запуск сервиса
 */
class CommandStartService extends CommandForService {
  public CommandStartService(String serviceName) {
    super("start_service", serviceName);
  }
}

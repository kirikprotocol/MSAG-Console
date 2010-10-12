package mobi.eyeline.informer.admin.service.daemon;

/**
 * Команда на остановку сервиса
 */
class CommandKillService extends CommandForService {
  public CommandKillService(String serviceName) {
    super("kill_service", serviceName);
  }
}

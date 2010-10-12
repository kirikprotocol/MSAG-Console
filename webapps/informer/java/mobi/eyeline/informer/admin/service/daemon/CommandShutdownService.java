package mobi.eyeline.informer.admin.service.daemon;

/**
 * Команда на остановку сервиса
 */
class CommandShutdownService extends CommandForService {
  public CommandShutdownService(String serviceName) {
    super("shutdown_service", serviceName);
  }
}

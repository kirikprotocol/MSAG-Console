package mobi.eyeline.informer.admin.service;

import mobi.eyeline.informer.admin.AdminException;

import java.io.File;
import java.util.Collection;

/**
 * API для запуска/остановки сервисов.
 *
 * @author Artem Snopkov
 */
public abstract class ServiceManager {

  public static ServiceManager getServiceManagerForSingleInst(String daemonHost, int daemonPort, File servicesDir) {
    return new ServiceManagerSingle(daemonHost, daemonPort, servicesDir);
  }

  public static ServiceManager getServiceManagerForHSInst(String daemonHost, int daemonPort, File servicesDir, Collection<String> daemonHosts) {
    return new ServiceManagerHS(daemonHost, daemonPort, servicesDir, daemonHosts);
  }

  public static ServiceManager getServiceManagerForHAInst(File resourcesFile, File servicesDir) throws AdminException {
    return new ServiceManagerHA(resourcesFile, servicesDir);
  }

  /**
   * Возвращает идентификаторы всех администрируемых сервисов
   *
   * @return идентификаторы всех администрируемых сервисов
   */
  public abstract Collection<String> getServices() throws AdminException;

  /**
   * Возвращает информацию о сервисе по его идентификатору
   *
   * @param service идентификатор сервиса
   * @return информацию о заданном сервисе или null, если сервис не найден
   */
  public abstract ServiceInfo getService(String service) throws AdminException;

  /**
   * Запускает сервис на заданном хосте по-умолчанию
   *
   * @param service название сервиса
   * @return true, если успех
   */
  public abstract void startService(String service) throws AdminException;

  /**
   * Останавливает сервис
   *
   * @param service идентификатор сервиса
   */
  public abstract void stopService(String service) throws AdminException;

  /**
   * Переключает сервис на заданный хост. Список всех хостов, на которых можно запустить сервис,
   * доступен через ServiceInfo.
   *
   * @param service идентификатор сервиса
   * @param toHost  название хоста, на который надо переключить сервис.
   */
  public abstract void swichService(String service, String toHost) throws AdminException;
}

package ru.novosoft.smsc.admin.service;

import ru.novosoft.smsc.admin.AdminException;

import java.util.Collection;

/**
 * API для запуска/остановки сервисов.
 * @author Artem Snopkov
 */
public abstract class ServiceManager {

  private static ServiceManager instance;

  public static synchronized ServiceManager getInstance() {

    if (instance == null) {
      // todo Инициализировать ServiceManagerHS, ServiceManagerHA или ServiceManagerSingle
      // todo перед этим надо понять откуда брать параметры для инициализации этих классов
    }
    return instance;
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
   * доступен через ServiceInterface.
   *
   * @param service идентификатор сервиса
   * @param toHost  название хоста, на который надо переключить сервис.
   */
  public abstract void swichService(String service, String toHost) throws AdminException;
}

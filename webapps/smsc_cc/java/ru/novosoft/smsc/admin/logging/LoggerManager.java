package ru.novosoft.smsc.admin.logging;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public interface LoggerManager {

  /**
   * Возвращает текущие настройки логирования
   *
   * @return текущие настройки логирования
   * @throws ru.novosoft.smsc.admin.AdminException
   *          если настройки не доступны
   */
  public LoggerSettings getSettings() throws AdminException;

  /**
   * Устанавливает новые настройки логирования
   *
   * @param settings новые настройки логирования
   * @throws AdminException если обновить настройки не удалось
   */
  public void updateSettings(LoggerSettings settings) throws AdminException;
}

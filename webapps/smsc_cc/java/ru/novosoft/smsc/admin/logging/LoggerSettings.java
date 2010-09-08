package ru.novosoft.smsc.admin.logging;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.CCLoggingInfo;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

/**
 * Настройки логирования
 * @author Artem Snopkov
 */
public class LoggerSettings {

  private final Map<String, Logger> loggers;

  LoggerSettings(Map<String, Logger> loggers) {
    this.loggers = loggers;
  }

  Map<String, Logger> getLoggers() {
    return loggers;
  }

  /**
   * Возвращает список доступных категорий
   * @return список доступных категорий
   */
  public Collection<String> getNames() {
    return loggers.keySet();
  }

  /**
   * Возвращает настройки логирования для указанной категории
   * @param name название категории
   * @return настройки логирования для указанной категории или null, если категория не существует
   */
  public Logger getLogger(String name) {
    return loggers.get(name);
  }

  /**
   * Обновляет настройки логирования для укказанной категории
   * @param name название категории
   * @param logger новые настройки логирования
   * @throws AdminException если категория не найдена или настройки не верны
   */
  public void updateLogger(String name, Logger logger) throws AdminException {
    if (name == null || logger == null)
      throw new NullPointerException();

    if (!loggers.containsKey(name))
      throw new LoggerException("category.not.found", name);

    loggers.put(name, new Logger(logger));
  }
}

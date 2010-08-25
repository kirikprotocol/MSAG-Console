package ru.novosoft.smsc.web.config;


/**
 * Структура, содержащая данные о последнем изменении конфигурации
 * @author Artem Snopkov
 */
public class UpdateInfo {
  
  private long lastUpdateTime;
  private String lastUpdateUser;
  private boolean changed;

  UpdateInfo(long lastUpdateTime, String lastUpdateUser, boolean changed) {
    this.lastUpdateTime = lastUpdateTime;
    this.lastUpdateUser = lastUpdateUser;
    this.changed = changed;
  }

  /**
   * Возвращает время последнего изменения или применения конфигурации
   * @return время последнего изменения конфигурации
   */
  public long getLastUpdateTime() {
    return lastUpdateTime;
  }

  /**
   * Возвращает логин пользователя, который внес последние изменения в конфигурацию или применил изменения
   * @return логин пользователя, который внес последние изменения в конфигурацию
   */
  public String getLastUpdateUser() {
    return lastUpdateUser;
  }

  /**
   * Возвращает true, если конфигурация в данный момент содержит непримененные изменения.
   * @return true, если конфигурация в данный момент содержит непримененные изменения.
   */
  public boolean isChanged() {
    return changed;
  }
}

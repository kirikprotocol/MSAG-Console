package ru.novosoft.smsc.admin.util;

/**
 * Класс для отслеживания прогресса
 * @author Artem Snopkov
 */
public interface ProgressObserver {

  /**
   * Метод периодически вызывается. Прогресс может быть вычислен так: current/total * 100%
   * @param current текущая позиция
   * @param total всего
   */
  public void update(long current, long total);
}

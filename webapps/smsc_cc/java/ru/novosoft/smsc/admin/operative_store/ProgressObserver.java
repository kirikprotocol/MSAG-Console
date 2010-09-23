package ru.novosoft.smsc.admin.operative_store;

/**
 * Класс для отслеживания прогресса при чтении оперативного стораджа
 * @author Artem Snopkov
 */
public interface ProgressObserver {

  /**
   * Метод периодически вызывается при чтении стораджа. Прогресс может быть вычислен так: current/total * 100%
   * @param current текущая позиция
   * @param total всего
   */
  public void update(long current, long total);
}

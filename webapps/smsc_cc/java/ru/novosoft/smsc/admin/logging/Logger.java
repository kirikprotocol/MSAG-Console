package ru.novosoft.smsc.admin.logging;

/**
 * Структура, описывающая настройки логирования для отдельной категории
 * @author Artem Snopkov
 */
public class Logger {

  private Level level;

  public Logger() {
  }

  public Logger(Logger copy) {
    this.level = copy.level;
  }

  /**
   * Возвращает уровень логирования или null, если уровень не задан
   * @return уровень логирования или null, если уровень не задан
   */
  public Level getLevel() {
    return level;
  }

  /**
   * Устанавливает уровень логирования
   * @param level новый уровень логирования
   */
  public void setLevel(Level level) {
    this.level = level;
  }

  public enum Level {
    DEBUG, INFO, WARN, ERROR, FATAL
  }
}

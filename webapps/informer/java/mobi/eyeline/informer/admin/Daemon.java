package mobi.eyeline.informer.admin;

/**
 * Демон (функция Informer)
 * @author Aleksandr Khalitov
 */
public interface Daemon {

  /**
   * Возвращает уникальное имя демона
   * @return имя
   */
  public String getName();

  /**
   * Запуск демона
   * @throws AdminException ошибка запуска
   */
  public void start() throws AdminException;

  /**
   * Остановка демона
   * @throws AdminException ошибка остановки
   */
  public void stop() throws AdminException;

  /**
   * Возвращает флаг, запущен ли демон
   * @return true - да, false - иначе
   */
  public boolean isStarted();

}

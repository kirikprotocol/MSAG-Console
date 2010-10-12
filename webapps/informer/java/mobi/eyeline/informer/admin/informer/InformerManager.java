package mobi.eyeline.informer.admin.informer;

import mobi.eyeline.informer.admin.AdminException;

import java.util.List;

/**
 * Управление Informer-ом
 * @author Aleksandr Khalitov
 */
public interface InformerManager{

  /**
   * Возвращает настройки Informer
   * @return настройки Informer
   * @throws AdminException ошибка чтения настроек
   */
  public InformerSettings getConfigSettings() throws AdminException;

  /**
   * Сохраняет настроки
   * @param informerSettings настройки
   * @throws AdminException ошибка сохранения
   */
  public void updateSettings(InformerSettings informerSettings) throws AdminException;


  /**
   * Запуск Informer
   * @throws AdminException ошибка при запуске
   */
  public void startInformer() throws AdminException;

  /**
   * Остановка Informer
   * @throws AdminException ошибка при остановке
   */
  public void stopInformer() throws AdminException;

  /**
   * Переключение Infromer на другую ноду
   * @param toHost хост
   * @throws AdminException ошибка при переключении
   */
  public void switchInformer(String toHost) throws AdminException;

  /**
   * Возвращает хост, на котором запущен Informer
   * @throws AdminException ошибка чтения статуса Informer
   * @return хост
   */
  public String getInformerOnlineHost() throws AdminException;

  /**
   * Возвращает список всех нод, на которых может быть запущен Informer
   * @throws AdminException ошибка чтения статуса Informer
   * @return список нод
   */
  public List<String> getInformerHosts() throws AdminException;
}

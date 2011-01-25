package mobi.eyeline.informer.admin.ftpserver;

import mobi.eyeline.informer.admin.AdminException;

import java.io.File;
import java.util.Collection;
import java.util.List;

/**
 * User: artem
 * Date: 25.01.11
 */
public interface FtpServerManager {

  /**
   * Возвращает базовую директорию для пользователя.
   *
   * @param login логин пользователя
   * @return базовую директорию для пользователя.
   * @throws AdminException если произошла ошибка
   */
  public File getUserHomeDir(String login) throws AdminException;

  /**
   * Обновляет данные о пользователях на сервере
   *
   * @param users список пользователей
   * @throws AdminException ошибка
   */
  public void updateUsers(Collection<FtpUser> users) throws AdminException;

  /**
   * Запуск сервера
   *
   * @throws AdminException ошибка запуска
   */
  public void startServer() throws AdminException;

  /**
   * Остановка сервера
   *
   * @throws AdminException ошибка при остановке
   */
  public void stopServer() throws AdminException;

  /**
   * Переключение сервера на другую ноду
   *
   * @param toHost хост
   * @throws AdminException ошибка при переключении
   */
  public void switchServer(String toHost) throws AdminException;

  /**
   * Возвращает хост, на котором запущен сервер или null, если сервер остановлен
   *
   * @return хост
   * @throws AdminException ошибка чтения статуса
   */
  public String getOnlineHost() throws AdminException;

  /**
   * Возвращает список всех нод, на которых может быть запущен сервер
   *
   * @return список нод
   * @throws AdminException ошибка чтения статуса
   */
  public List<String> getServerHosts() throws AdminException;
}

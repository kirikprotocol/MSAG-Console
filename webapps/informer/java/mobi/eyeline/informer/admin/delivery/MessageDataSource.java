package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Сущность для обрабоки сообщений
 *
 * @author Aleksandr Khalitov
 */
public interface MessageDataSource {

  /**
   * Возвращает следующее сообщение для обработки. Если сообщений больше нет, то возвращает null
   *
   * @return сообщение для обработки, если оно есть, null иначе
   * @throws AdminException ошибка чтения сообщений
   */
  public Message next() throws AdminException;

}

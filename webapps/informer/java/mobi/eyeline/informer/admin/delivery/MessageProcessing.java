package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Сущность для обрабоки сообщений
 * @author Aleksandr Khalitov
 */
public interface MessageProcessing {

  /**
   * Возвращает следующее сообщение для обработки. Если сообщений больше нет, то возвращает null
   * @return сообщение для обработки, если оно есть, null иначе
   * @throws AdminException ошибка чтения сообщений
   */
  public Message getToProcess() throws AdminException;

  /**
   * Сигнализирует о том, что все сообщения успешно обработаны).
   */
  public void success();

  /**
   * Сигнализирует о том, что обработка сообщений закончилась неудачей).
   * @param error ошибка при обработки
   */
  public void error(Throwable error);

  /**
   * Увеливает счётчик обработанных сообщений
   * @param count кол-во обработанных сообщений за период после предыдущего вызова
   */
  public void incrementProcessed(int count);

}

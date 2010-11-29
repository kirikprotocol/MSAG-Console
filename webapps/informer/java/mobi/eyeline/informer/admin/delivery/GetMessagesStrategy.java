package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Интерфейс для стратегии извлечения сообщений рассылки из Информера.
 *
 * Проблема в том, что Информер в ответ на запрос requestMessageStates возвращает текущие состояния сообщений,
 * а кусок Activity Log-а, где для каждого сообщения может быть несколько записей. Когда сообщение добавляется
 * в информер в Activity Log-е появляется запись о том, что сообщение в статусе NEW. Далее, перед отправкой в
 * логе снова появляется запись, что сообщение теперь в статусе Process. Наконец, после доставки в логе появляется
 * запись о том, что сообщение в статусе Delivered (или Failed или Expired).
 *
 * В тоже время, интерфейс DeliveryManager-а предполагает, что метод getMessages возвращает текущие состояния сообщений,
 * и там по 1 записи для каждого сообщения.
 *
 * GetMessagesStrategy описывает алгоритм извлечения сообщений из Информера.
 *
 * @author Artem Snopkov
 */
interface GetMessagesStrategy {

  /**
   * Алгоритм извлечения сообщений
   * @param conn коннект к информеру
   * @param filter фильтр сообщений
   * @param _pieceSize размер страницы
   * @param visitor визитор, в который надо передать сообщения
   * @throws AdminException если произошла ошибка
   */
  public void getMessages(DcpConnection conn, MessageFilter filter, int _pieceSize, final Visitor<Message> visitor) throws AdminException;

  /**
   * Алгоритм подсчета числа сообщений
   * @param conn коннект к информеру
   * @param messageFilter фильтр
   * @return количество сообщений
   * @throws AdminException если произошла ошибка
   */
  public int countMessages(DcpConnection conn, MessageFilter messageFilter) throws AdminException;
}

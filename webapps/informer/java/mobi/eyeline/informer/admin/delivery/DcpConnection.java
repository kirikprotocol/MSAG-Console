package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

import java.util.Collection;

/**
 * Коннект к DCP, упрвление рассылками
 *
 * @author Aleksandr Khalitov
 */
public interface DcpConnection {

  /**
   * Соединение установлено
   *
   * @return true - да, false - нет
   */
  public boolean isConnected();

  /**
   * Закрывает соединение
   */
  public void close();

  /**
   * Создание рассылки
   *
   * @param delivery рассылка
   * @return идентификатор рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public int createDelivery(Delivery delivery) throws AdminException;


  /**
   * Добавляет сообщения в рассылку
   *
   * @param deliveryId идентификатор рассылки
   * @param messages   сообщения
   * @return идентификаторы сообщений
   * @throws AdminException ошибка выполнения команды
   */
  public long[] addDeliveryMessages(int deliveryId, Message[] messages) throws AdminException;

  /**
   * Модификация рассылки
   *
   * @param delivery рассылка
   * @throws AdminException ошибка выполнения команды
   */
  public void modifyDelivery(Delivery delivery) throws AdminException;

  /**
   * Удаление рассылки
   *
   * @param deliveryId идентификатор рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public void dropDelivery(int deliveryId) throws AdminException;

  /**
   * Подсчёт кол-ва рассылок
   *
   * @param deliveryFilter фильтр
   * @return кол-во рассылок
   * @throws AdminException ошибка выполнения команды
   */
  public int countDeliveries(DeliveryFilter deliveryFilter) throws AdminException;

  /**
   * Удаляет сообщения из рассылки
   *
   * @param messageIds идентификаторы сообщений
   * @throws AdminException ошибка выполнения команды
   */
  public void dropMessages(long[] messageIds) throws AdminException;

  /**
   * Возвращает рассылку по идентификатору
   *
   * @param deliveryId идентификатор рассылки
   * @return рассылка
   * @throws AdminException ошибка выполнения команды
   */
  public Delivery getDelivery(int deliveryId) throws AdminException;

  /**
   * Меняет состояние рассылки
   *
   * @param deliveryId идентификатор рассылки
   * @param state      новое состояние
   * @throws AdminException ошибка выполнения команды
   */
  public void changeDeliveryState(int deliveryId, DeliveryState state) throws AdminException;

  /**
   * Возвращает статистику по рассылке
   *
   * @param deliveryId идентификатор рассылки
   * @return статистика по рассылке
   * @throws AdminException ошибка выполнения команды
   */
  public DeliveryStatistics getDeliveryState(int deliveryId) throws AdminException;

  /**
   * Возвращает идентификатор запроса для извлечения рассылок, удовлетворяющие фильтру
   *
   * @param deliveryFilter фильтр
   * @return идентификатор запроса
   * @throws AdminException ошибка выполнения команды
   */
  public int getDeliviries(DeliveryFilter deliveryFilter) throws AdminException;

  /**
   * Возвращает следующую часть рассылок по идентификатору запроса
   *
   * @param reqId      идентификатор запроса
   * @param pieceSize  максимальное кол-во извлекаемых рассылок
   * @param deliveries куда следуют сложить рассылки
   * @return есть ли ещё рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public boolean getNextDeliviries(int reqId, int pieceSize, Collection<DeliveryInfo> deliveries) throws AdminException;

  /**
   * Возвращает идентифкатор запроса для извлечения информации о сообщениях рассылки
   *
   * @param filter фильтр
   * @return идентифкатор запроса
   * @throws AdminException ошибка выполнения команды
   */
  public int getMessagesStates(MessageFilter filter) throws AdminException;

  /**
   * Возвращает следующую часть сообщений по идентификатору запроса
   *
   * @param reqId     идентификатор запроса
   * @param pieceSize максимальное кол-во извлекаемых сообщений
   * @param messages  куда следуют сложить сообщения
   * @return есть ли ещё сообщения
   * @throws AdminException ошибка выполнения команды
   */
  public boolean getNextMessageStates(int reqId, int pieceSize, Collection<mobi.eyeline.informer.admin.delivery.MessageInfo> messages) throws AdminException;


  /**
   * Подсчёт кол-ва сообщений
   *
   * @param messageFilter фильтр
   * @return кол-во сообщений
   * @throws AdminException ошибка выполнения команды
   */
  public int countMessages(MessageFilter messageFilter) throws AdminException;

  /**
   * Возвращает история статусов рассылки
   * @param deliveryId идентификатор рассылки
   * @return история статусов рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public DeliveryHistory getDeliveryHistory(int deliveryId) throws AdminException;

}

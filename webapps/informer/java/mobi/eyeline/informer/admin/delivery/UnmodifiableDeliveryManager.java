package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
public interface UnmodifiableDeliveryManager {

  /**
   * Подсчёт кол-ва рассылок
   *
   * @param login          логин
   * @param password       пароль
   * @param deliveryFilter фильтр
   * @return кол-во рассылок
   * @throws AdminException ошибка выполнения команды
   */
  public int countDeliveries(String login, String password, DeliveryFilter deliveryFilter) throws AdminException;


  /**
   * Возвращает рассылку по идентификатору
   *
   * @param login      логин
   * @param password   пароль
   * @param deliveryId идентификатор рассылки
   * @return рассылка
   * @throws AdminException ошибка выполнения команды
   */
  public Delivery getDelivery(String login, String password, int deliveryId) throws AdminException;

  /**
   * Возвращает рассылки, удовлетворяющие фильтру
   *
   * @param login          логин
   * @param password       пароль
   * @param deliveryFilter фильтр
   * @param _pieceSize     сколько рассылок извлекать за одну транзакцию
   * @param visitor        визитер извлечения рассылок
   * @throws AdminException ошибка выполнения команды
   */
  public void getDeliveries(final String login, final String password, DeliveryFilter deliveryFilter, int _pieceSize, Visitor<Delivery> visitor) throws AdminException;


  /**
   * Возвращает информацию о сообщениях рассылки
   *
   * @param login      логин
   * @param password   пароль
   * @param filter     фильтр
   * @param _pieceSize сколько рассылок извлекать за одну транзакцию
   * @param visitor    визитер извлечения сообщений
   * @throws AdminException ошибка выполнения команды
   */
  public void getMessages(String login, String password, MessageFilter filter, int _pieceSize, Visitor<Message> visitor) throws AdminException;

  /**
   * Подсчёт кол-ва сообщений
   *
   * @param login         логин
   * @param password      пароль
   * @param messageFilter фильтр
   * @return кол-во сообщений
   * @throws AdminException ошибка выполнения команды
   */
  public int countMessages(String login, String password, MessageFilter messageFilter) throws AdminException;

}

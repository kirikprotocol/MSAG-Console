package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Коннект к DCP, упрвление рассылками 
 * @author Aleksandr Khalitov
 */
public interface DcpConnection {

  /**
   * Соединение установлено
   * @return true - да, false - нет
   */
  public boolean isConnected();

  /**
   * Создание рассылки
   * @param delivery рассылка
   * @return идентификатор рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public int createDelivery(Delivery delivery) throws AdminException;

  /**
   * Модификация рассылки
   * @param delivery рассылка
   * @throws AdminException ошибка выполнения команды
   */
  public void modifyDelivery(Delivery delivery) throws AdminException;

  /**
   * Удаление рассылки
   * @param deliveryId идентификатор рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public void dropDelivery(int deliveryId) throws AdminException;

  /**
   * Подсчёт кол-ва рассылок
   * @param deliveryFilter фильтр
   * @throws AdminException ошибка выполнения команды
   * @return кол-во рассылок
   */
  public int countDeliveries(DeliveryFilter deliveryFilter) throws AdminException;

  /**
   * Удаляет сообщения из рассылки
   * @param messageIds идентификаторы сообщений
   * @throws AdminException ошибка выполнения команды
   */
  public void dropMessages(long ... messageIds) throws AdminException;

  /**
   * Возвращает глоссарий по рассылке
   * @param deliveryId идентикатор рассылки
   * @return глоссарий
   * @throws AdminException ошибка выполнения команды
   */
  public String[] getDeliveryGlossary(int deliveryId) throws AdminException;

  /**
   * Добавляет глоссарий к рассылке
   * @param deliveryId идентикатор рассылки
   * @param messages глоссарий
   * @throws AdminException ошибка выполнения команды
   */
  public void modifyDeliveryGlossary(int deliveryId, String[] messages) throws AdminException;

  /**
   * Возвращает рассылку по идентификатору
   * @param deliveryId идентификатор рассылки
   * @return рассылка
   * @throws AdminException ошибка выполнения команды
   */
  public Delivery getDelivery(int deliveryId) throws AdminException;

}

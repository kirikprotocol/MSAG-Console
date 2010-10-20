package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryManager {

  private final DcpConnectionFactory connectionFactory;
  
  public DeliveryManager(String host, int port) {
    this.connectionFactory = new DcpConnectionFactory(host, port);
  }

  protected DeliveryManager(DcpConnectionFactory factory) {
    this.connectionFactory = factory;
  }

  /**
   * Создание рассылки
   * @param delivery рассылка
   * @throws mobi.eyeline.informer.admin.AdminException ошибка выполнения команды
   */
  public void createDelivery(Delivery delivery) throws AdminException {

  }

  /**
   * Модификация рассылки
   * @param delivery рассылка
   * @throws AdminException ошибка выполнения команды
   */
  public void modifyDelivery(Delivery delivery) throws AdminException {

  }

  /**
   * Удаление рассылки
   * @param deliveryId идентификатор рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public void dropDelivery(int deliveryId) throws AdminException {

  }

  /**
   * Подсчёт кол-ва рассылок
   * @param deliveryFilter фильтр
   * @throws AdminException ошибка выполнения команды
   * @return кол-во рассылок
   */
  public int countDeliveries(DeliveryFilter deliveryFilter) throws AdminException {
    return 0;
  }

  /**
   * Удаляет сообщения из рассылки
   * @param messageIds идентификаторы сообщений
   * @throws AdminException ошибка выполнения команды
   */
  public void dropMessages(long ... messageIds) throws AdminException {

  }

  /**
   * Возвращает глоссарий по рассылке
   * @param deliveryId идентикатор рассылки
   * @return глоссарий
   * @throws AdminException ошибка выполнения команды
   */
  public String[] getDeliveryGlossary(int deliveryId) throws AdminException {
    return null;
  }

  /**
   * Добавляет глоссарий к рассылке
   * @param deliveryId идентикатор рассылки
   * @param messages глоссарий
   * @throws AdminException ошибка выполнения команды
   */
  public void modifyDeliveryGlossary(int deliveryId, String[] messages) throws AdminException {

  }

  /**
   * Возвращает рассылку по идентификатору
   * @param deliveryId идентификатор рассылки
   * @return рассылка
   * @throws AdminException ошибка выполнения команды
   */
  public Delivery getDelivery(int deliveryId) throws AdminException {
    return null;
  }

  /**
   * Меняет состояние рассылки
   * @param deliveryId идентификатор рассылки
   * @param state новое состояние
   * @throws AdminException ошибка выполнения команды
   */
  public void changeDeliveryState (int deliveryId, DeliveryState state) throws AdminException {

  }

  /**
   * Возвращает статистику по рассылке
   * @param deliveryId идентификатор рассылки
   * @return статистика по рассылке
   * @throws AdminException ошибка выполнения команды
   */
  public DeliveryStatistics getDeliveryState(int deliveryId) throws AdminException {
    return null;
  }

  /**
   * Возвращает рассылки, удовлетворяющие фильтру
   * @param deliveryFilter фильтр
   * @return рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public DeliveryDataSource<DeliveryInfo> getDeliviries(DeliveryFilter deliveryFilter) throws AdminException {
    return null;
  }



  /**
   * Возвращает информацию о сообщениях рассылки
   * @param filter фильтр
   * @return информация о сообщениях рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public DeliveryDataSource<MessageInfo> getMessagesStates(MessageFilter filter) throws AdminException {
    return null;
  }

}

package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DeliveryPrototype;

/**
 * Создатель рассылок
 *
 * @author Artem Snopkov
 */
interface DeliveryBuilder {

  /**
   * Основной метод, позволяющий создать рассылку.
   * @param proto прототип рассылки
   * @throws DeliveryControllerException усли произошла ошибка
   */
  void createDelivery(DeliveryPrototype proto) throws DeliveryControllerException;

  // Счетчики для определения прогресса

  int getCurrent();

  int getTotal();

  /**
   * Количество загруженных сообщений
   * @return количество загруенных сообщений
   */
  int getProcessed();

  /**
   * Прерывает процесс создания рассылки
   */
  void cancelDeliveryCreation() ;

  /**
   * Удаляет созданные рассылки, если они есть
   * @throws AdminException если произошла ошибка
   */
  void removeDelivery() throws AdminException;

  /**
   * Метод должен быть вызван после окончания использования Builder-f
   * @throws AdminException если произошла ошибка
   */
  void shutdown() throws AdminException;

}

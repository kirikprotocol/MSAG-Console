package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Стратегия для обработки рассылок на удалённом ресурсе
 * @author Aleksandr Khalitov
 */
interface ResourceProcessStrategy {

  /**
   * Обработка рассылок на удаленном ресерсе

   * @param allowDeliveryCreation разрешено ли создавать рассылки
   * @throws AdminException ошибка
   */
  public void process(boolean allowDeliveryCreation) throws AdminException;

}

package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;

import java.io.File;

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

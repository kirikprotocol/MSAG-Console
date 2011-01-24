package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;

import java.io.File;

/**
 * Стратегия для обработки рассылок на удалённом ресурсе
 * @author Aleksandr Khalitov
 */
public interface ResourceProcessStrategy {

  /**
   * Обработка рассылок на удаленном ресерсе
   * @param u пользователь
   * @param userDir директория пользователя на локальной машине
   * @param ucps настройки ресурса
   * @throws AdminException ошибка
   */
  public void process(User u, File userDir, UserCPsettings ucps) throws AdminException;

}

package mobi.eyeline.informer.admin.blacklist;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;

import java.util.Collection;

/**
 * Управление запрещённые номера
 *
 * @author Aleksandr Khalitov
 */
public interface BlacklistManager {

  /**
   * Добавление номера в список запрещённых
   *
   * @param msisdn номер
   * @throws AdminException ошибка при добавлении
   */
  public void add(Address msisdn) throws AdminException;

  /**
   * Добавление нескольких номеров в список запрещённых
   *
   * @param msisdns номера
   * @throws AdminException ошибка при добавлении
   */
  public void add(Collection<Address> msisdns) throws AdminException;

  /**
   * Удаление номера из списка запрещённых
   *
   * @param msisdn номер
   * @throws AdminException ошибка при удалении
   */
  public void remove(Address msisdn) throws AdminException;

  /**
   * Удаление нескольких номераов из списка запрещённых
   *
   * @param msisdns номера
   * @throws AdminException ошибка при удалении
   */
  public void remove(Collection<Address> msisdns) throws AdminException;

  /**
   * Проверка принадлежит ли номер списку заперщённых
   *
   * @param msisdn номер
   * @return true - да, false - иначе
   * @throws AdminException ошибка при чтении списка
   */
  public boolean contains(Address msisdn) throws AdminException;

}

package mobi.eyeline.informer.admin.blacklist;

import mobi.eyeline.informer.admin.AdminException;

import java.util.Collection;

/**
 * Управление запрещённые номера
 * @author Aleksandr Khalitov
 */
public interface BlacklistManager {

  /**
   * Добавление номера в список запрещённых
   * @param msisdn номер
   * @throws AdminException ошибка при добавлении
   */
  public void add(String msisdn) throws AdminException;

  /**
   * Добавление нескольких номеров в список запрещённых
   * @param msisdns номера
   * @throws AdminException ошибка при добавлении
   */
  public void add(Collection<String> msisdns) throws AdminException;

  /**
   * Удаление номера из списка запрещённых
   * @param msisdn номер
   * @throws AdminException ошибка при удалении
   */
  public void remove(String msisdn) throws AdminException;     

  /**
   * Удаление нескольких номераов из списка запрещённых
   * @param msisdns номера
   * @throws AdminException ошибка при удалении
   */
  public void remove(Collection<String> msisdns) throws AdminException;

  /**
   * Проверка принадлежит ли номер списку заперщённых
   * @param msisdn номер
   * @return true - да, false - иначе
   * @throws AdminException ошибка при чтении списка
   */
  public boolean contains(String msisdn) throws AdminException;

}

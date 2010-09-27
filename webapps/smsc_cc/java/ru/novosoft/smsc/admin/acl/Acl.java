package ru.novosoft.smsc.admin.acl;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Address;

import java.io.Serializable;
import java.util.List;

/**
 * Класс, описывающий Access Controll List
 *
 * @author Artem Snopkov
 */
public interface Acl extends Serializable {

  /**
   * Обновляет данные об ACL
   *
   * @param name        новое имя
   * @param description новое описание
   * @throws AdminException если произошла ошибка
   */
  public void updateInfo(String name, String description) throws AdminException;

  /**
   * Возвращает список адресов, содержащихся в данном ACL
   *
   * @return список адресов, содержащихся в данном ACL
   * @throws AdminException если произошла ошибка
   */
  public List<Address> getAddresses() throws AdminException;

  /**
   * Добавляет новые адреса в ACL
   *
   * @param addresses список адресов, которые надо добавить в ACL
   * @throws AdminException если произошла ошибка
   */
  public void addAddresses(List<Address> addresses) throws AdminException;

  /**
   * Удаляет адреса из ACL
   *
   * @param addresses список адресов, которые надо удалить из ACL
   * @throws AdminException если произошла ошибка
   */
  public void removeAddresses(List<Address> addresses) throws AdminException;

  /**
   * Возвращает идентификатор ACL
   *
   * @return идентификатор ACL
   */
  public int getId();

  /**
   * Возвращает имя ACL
   *
   * @return имя ACL
   */
  public String getName();

  /**
   * Возвращает описание ACL
   *
   * @return описание ACL
   */
  public String getDescription();

}

package ru.novosoft.smsc.admin.acl;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;
import ru.novosoft.smsc.util.Address;

import java.io.Serializable;
import java.util.List;

/**
 * Класс, описывающий Access Controll List
 *
 * @author Artem Snopkov
 */
public class Acl implements Serializable {

  static final int MAX_NAME_LEN = 31;
  static final int MAX_DESCRIPTION_LEN = 127;

  private static final transient ValidationHelper vh = new ValidationHelper(Acl.class);

  private final int id;
  private transient final AclManager manager;
  private String name;
  private String description;

  Acl(AclManager manager, int id, String name, String description) {
    this.id = id;
    this.manager = manager;
    this.name = name;
    this.description = description;
  }

  static void checkInfo(String name, String description) throws AdminException {
    vh.checkNotEmpty("name", name);
    vh.checkLen("name", name, 0, MAX_NAME_LEN);
    if (description != null)
      vh.checkLen("description", description, 0, MAX_DESCRIPTION_LEN);
  }

  static void checkAddresses(List<Address> addresses) throws AdminException {
    vh.checkNoNulls("addresses", addresses);
  }

  /**
   * Обновляет данные об ACL
   *
   * @param name        новое имя
   * @param description новое описание
   * @throws AdminException если произошла ошибка
   */
  public void updateInfo(String name, String description) throws AdminException {
    checkInfo(name, description);
    manager.updateAcl(id, name, description);
    this.name = name;
    this.description = description;
  }

  /**
   * Возвращает список адресов, содержащихся в данном ACL
   *
   * @return список адресов, содержащихся в данном ACL
   * @throws AdminException если произошла ошибка
   */
  public List<Address> getAddresses() throws AdminException {
    return manager.getAddresses(id);
  }

  /**
   * Добавляет новые адреса в ACL
   *
   * @param addresses список адресов, которые надо добавить в ACL
   * @throws AdminException если произошла ошибка
   */
  public void addAddresses(List<Address> addresses) throws AdminException {
    checkAddresses(addresses);
    manager.addAddresses(id, addresses);
  }

  /**
   * Удаляет адреса из ACL
   *
   * @param addresses список адресов, которые надо удалить из ACL
   * @throws AdminException если произошла ошибка
   */
  public void removeAddresses(List<Address> addresses) throws AdminException {
    checkAddresses(addresses);
    manager.removeAddresses(id, addresses);
  }

  /**
   * Возвращает идентификатор ACL
   *
   * @return идентификатор ACL
   */
  public int getId() {
    return id;
  }

  /**
   * Возвращает имя ACL
   *
   * @return имя ACL
   */
  public String getName() {
    return name;
  }

  /**
   * Возвращает описание ACL
   *
   * @return описание ACL
   */
  public String getDescription() {
    return description;
  }
}

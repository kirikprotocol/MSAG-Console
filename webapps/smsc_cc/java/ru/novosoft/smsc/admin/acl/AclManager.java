package ru.novosoft.smsc.admin.acl;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfiguration;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.util.Address;

import java.util.*;

/**
 * @author Artem Snopkov
 */
public interface AclManager extends SmscConfiguration {

  /**
   * Создает и возвращает новый ACL
   *
   * @param name        имя нового ACL
   * @param description описание нового ACL
   * @param addresses   список адресов, содержащихся в новом ACL, или null, если таких нет
   * @throws AdminException если произошла ошибка
   * @return новый экземпляр ACL
   */
  public Acl createAcl(String name, String description, List<Address> addresses) throws AdminException;

  /**
   * Удаляет ACL с заданным идентификатором из списка
   *
   * @param aclId идентификатор ACL, который надо удалить
   * @throws AdminException если произошла ошибка
   */
  public void removeAcl(int aclId) throws AdminException;

  /**
   * Возвращает ACL по его идентификатору
   *
   * @param id идентификатор ACL
   * @return ACL с заданным идентификатором
   * @throws AdminException если произошла ошибка
   */
  public Acl getAcl(int id) throws AdminException;

  /**
   * Возвращает список всех ACL
   *
   * @return список всех ACL
   * @throws AdminException если произошла ошибка
   */
  public List<Acl> acls() throws AdminException;

  /**
   * Проверяет статус ACL во всех инстанцах СМСЦ
   *
   * @return статус ACL во всех инстанцах СМСЦ или null, если информация недоступна
   * @throws AdminException если произошла ошибка при выполнении операции
   */
  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException;

}

package ru.novosoft.smsc.admin.acl;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.CCAclInfo;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.config.SmscConfiguration;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.util.ValidationHelper;
import ru.novosoft.smsc.util.Address;

import java.util.*;

/**
 * @author Artem Snopkov
 */
public class AclManager implements SmscConfiguration {

  private final ClusterController cc;

  public AclManager(ClusterController cc) {
    this.cc = cc;
  }

  private synchronized int getNextAclId() throws AdminException {
    int maxAclId = -1;
    for (CCAclInfo acl : cc.getAcls())
      maxAclId = Math.max(maxAclId, acl.getId());
    return maxAclId+1;
  }

  /**
   * Создает и возвращает новый ACL
   * @param name имя нового ACL
   * @param description описание нового ACL
   * @param addresses список адресов, содержащихся в новом ACL, или null, если таких нет
   * @throws AdminException если произошла ошибка
   */
  public Acl createAcl(String name, String description, List<Address> addresses) throws AdminException {
    Acl.checkInfo(name, description);
    if (addresses != null)
      Acl.checkAddresses(addresses);
    else
      addresses = Collections.emptyList();

    if (!cc.isOnline())
      throw new AclException("acls.not.available");

    int nextAclId = getNextAclId();
    cc.createAlc(nextAclId, name, description, addresses);
    return new Acl(this, nextAclId, name, description);
  }

  /**
   * Удаляет ACL с заданным идентификатором из списка
   * @param aclId идентификатор ACL, который надо удалить
   * @throws AdminException если произошла ошибка
   */
  public void removeAcl(int aclId) throws AdminException {
    if (!cc.isOnline())
      throw new AclException("acls.not.available");
    cc.removeAcl(aclId);
  }

  /**
   * Возвращает ACL по его идентификатору
   * @param id идентификатор ACL
   * @return ACL с заданным идентификатором
   * @throws AdminException если произошла ошибка
   */
  public Acl getAcl(int id) throws AdminException {
    if (!cc.isOnline())
      throw new AclException("acls.not.available");

    CCAclInfo acl =  cc.getAcl(id);
    return new Acl(this, acl.getId(), acl.getName(), acl.getDescription());
  }

  /**
   * Возвращает список всех ACL
   * @return список всех ACL
   * @throws AdminException если произошла ошибка
   */
  public List<Acl> acls() throws AdminException {
    if (!cc.isOnline())
      throw new AclException("acls.not.available");

    List<CCAclInfo> acls = cc.getAcls();
    ArrayList<Acl> result = new ArrayList<Acl>(acls.size() + 1);
    for (CCAclInfo acl : acls)
      result.add(new Acl(this, acl.getId(), acl.getName(), acl.getDescription()));
    return result;
  }

  /**
   * Проверяет статус ACL во всех инстанцах СМСЦ
   * @return статус ACL во всех инстанцах СМСЦ или null, если информация недоступна
   * @throws AdminException если произошла ошибка при выполнении операции
   */
  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    if (!cc.isOnline())
      return null;

    ConfigState state = cc.getAclState();
    long lastUpdate = state.getCcLastUpdateTime();
    Map<Integer, SmscConfigurationStatus> result = new HashMap<Integer, SmscConfigurationStatus>();
    for (Map.Entry<Integer, Long> e : state.getInstancesUpdateTimes().entrySet()) {
      SmscConfigurationStatus s = (e.getValue() >= lastUpdate) ? SmscConfigurationStatus.UP_TO_DATE : SmscConfigurationStatus.OUT_OF_DATE;
      result.put(e.getKey(), s);
    }
    return result;
  }

  void updateAcl(int id, String name, String description) throws AdminException {
    if (!cc.isOnline())
      throw new AclException("acls.not.available");

    cc.updateAcl(new CCAclInfo(id, name, description));
  }

  void addAddresses(int aclId, List<Address> addresses) throws AdminException {
    if (!cc.isOnline())
      throw new AclException("acls.not.available");

    cc.addAddressesToAcl(aclId, addresses);
  }

  void removeAddresses(int aclId, List<Address> addresses) throws AdminException {
    if (!cc.isOnline())
      throw new AclException("acls.not.available");

    cc.removeAddressesFromAcl(aclId, addresses);
  }

  List<Address> getAddresses(int aclId) throws AdminException {
    if (!cc.isOnline())
      throw new AclException("acls.not.available");

    return cc.getAclAddresses(aclId);
  }
}

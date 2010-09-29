package ru.novosoft.smsc.admin.acl;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.CCAclInfo;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.util.Address;

import java.util.*;

/**
 * @author Artem Snopkov
 */
public class AclManagerImpl implements AclManager {
  private final ClusterController cc;

  public AclManagerImpl(ClusterController cc) {
    this.cc = cc;
  }

  private synchronized int getNextAclId() throws AdminException {
    int maxAclId = -1;
    for (CCAclInfo acl : cc.getAcls())
      maxAclId = Math.max(maxAclId, acl.getId());
    return maxAclId + 1;
  }

  public Acl createAcl(String name, String description, List<Address> addresses) throws AdminException {
    AclImpl.checkInfo(name, description);
    if (addresses != null)
      AclImpl.checkAddresses(addresses);
    else
      addresses = Collections.emptyList();

    if (!cc.isOnline())
      throw new AclException("acls.not.available");

    int nextAclId = getNextAclId();
    cc.createAlc(nextAclId, name, description, addresses);
    return new AclImpl(this, nextAclId, name, description);
  }

  public void removeAcl(int aclId) throws AdminException {
    if (!cc.isOnline())
      throw new AclException("acls.not.available");
    cc.removeAcl(aclId);
  }

  public Acl getAcl(int id) throws AdminException {
    if (!cc.isOnline())
      throw new AclException("acls.not.available");

    CCAclInfo acl = cc.getAcl(id);
    return new AclImpl(this, acl.getId(), acl.getName(), acl.getDescription());
  }

  public List<Acl> acls() throws AdminException {
    if (!cc.isOnline())
      throw new AclException("acls.not.available");

    List<CCAclInfo> acls = cc.getAcls();
    ArrayList<Acl> result = new ArrayList<Acl>(acls.size() + 1);
    for (CCAclInfo acl : acls)
      result.add(new AclImpl(this, acl.getId(), acl.getName(), acl.getDescription()));
    return result;
  }
  
  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    if (!cc.isOnline())
      return null;

    ConfigState state = cc.getAclState();
    Map<Integer, SmscConfigurationStatus> result = new HashMap<Integer, SmscConfigurationStatus>();
    if (state != null) {
      long lastUpdate = state.getCcLastUpdateTime();
      for (Map.Entry<Integer, Long> e : state.getInstancesUpdateTimes().entrySet()) {
        SmscConfigurationStatus s = (e.getValue() >= lastUpdate) ? SmscConfigurationStatus.UP_TO_DATE : SmscConfigurationStatus.OUT_OF_DATE;
        result.put(e.getKey(), s);
      }
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

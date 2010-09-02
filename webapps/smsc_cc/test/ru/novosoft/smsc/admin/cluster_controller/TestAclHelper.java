package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Address;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
class TestAclHelper {

  private final Map<Integer, CCAcl> acls = new HashMap<Integer, CCAcl>();

  TestAclHelper() {
    acls.put(1, new CCAcl(new CCAclInfo(1, "first", "first description"), "+79139495113", "+79163516553"));
    acls.put(2, new CCAcl(new CCAclInfo(2, "second", "second description"), "+79139495114", "+79163516554"));
  }

  public void createAcl(int aclId, String name, String description, List<Address> addresses) throws AdminException {
    if (acls.containsKey(aclId))
      throw new ClusterControllerException("interaction_error");

    acls.put(aclId, new CCAcl(new CCAclInfo(aclId, name, description), addresses));
  }

  public void updateAcl(CCAclInfo info) throws ClusterControllerException {
    CCAcl acl = acls.get(info.getId());
    if (acl == null)
      throw new ClusterControllerException("interaction_error");

    acl.updateInfo(info);
  }

  public void removeAcl(int aclId) throws ClusterControllerException {
    if (!acls.containsKey(aclId))
      throw new ClusterControllerException("interaction_error");
    acls.remove(aclId);
  }

  public void addAddresses(int aclId, List<Address> addresses) throws ClusterControllerException {
    CCAcl acl = acls.get(aclId);
    if (acl == null)
      throw new ClusterControllerException("interaction_error");

    acl.addAddresses(addresses);
  }

  public void removeAddresses(int aclId, List<Address> addresses) throws ClusterControllerException {
    CCAcl acl = acls.get(aclId);
    if (acl == null)
      throw new ClusterControllerException("interaction_error");

    acl.removeAddresses(addresses);
  }

  public List<CCAclInfo> getAcls() {
    List<CCAclInfo> result = new ArrayList<CCAclInfo>();
    for (CCAcl acl : acls.values())
      result.add(acl.getInfo());
    return result;
  }

  public List<Address> getAclAddresses(int aclId) throws ClusterControllerException {
    CCAcl acl = acls.get(aclId);
    if (acl == null)
      throw new ClusterControllerException("interaction_error");
    return acl.getAddresses();
  }

  public CCAclInfo getAcl(int aclId) throws ClusterControllerException {
    CCAcl acl = acls.get(aclId);
    if (acl == null)
      throw new ClusterControllerException("interaction_error");
    return acl.getInfo();
  }
}

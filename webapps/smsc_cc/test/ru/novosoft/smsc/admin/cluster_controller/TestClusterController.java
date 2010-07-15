package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Address;

import java.util.Collection;

/**
 * @author Artem Snopkov
 */
public class TestClusterController extends ClusterController {

  public boolean applyRescheduleCalled;
  public boolean applyFraudCalled;
  public boolean applyMapLimitsCalled;
  public boolean applySnmpCalled;

  public void applyReschedule() throws AdminException {
    applyRescheduleCalled = true;
  }

  public void applyFraud() throws AdminException {
    applyFraudCalled = true;
  }

  public void applyMapLimits() throws AdminException {
    applyMapLimitsCalled = true;
  }

  public void applySnmp() {
    applySnmpCalled = true;
  }

  public boolean isOnline() {
    return true;
  }

  public void addAlias(String address, String alias, boolean aliasHide) throws AdminException {
  }

  public void delAlias(String alias) throws AdminException {
  }

  public void addClosedGroup(long groupId, String groupName, Collection<Address> masks) throws AdminException {
  }

  public void removeClosedGroup(long groupId) throws AdminException {
  }

  public void addMasksToClosedGroup(long groupId, Collection<Address> masks) throws AdminException {
  }

  public void removeMasksFromClosedGroup(long groupId, Collection<Address> masks) throws AdminException {
  }
}

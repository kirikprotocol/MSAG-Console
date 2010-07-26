package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Address;

import java.util.HashMap;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class TestClusterController extends ClusterController {

  public boolean applyRescheduleCalled;
  public boolean applyFraudCalled;
  public boolean applyMapLimitsCalled;
  public boolean applySnmpCalled;

  private int smscInstancesNumber;

  public TestClusterController(int smscInstancesNumber) {
    this.smscInstancesNumber = smscInstancesNumber;
  }

  public TestClusterController() {
    this(2);
  }

  public ConfigState getAliasesConfigState() throws AdminException {
    long now = System.currentTimeMillis();
    Map<Integer, Long> map = new HashMap<Integer, Long>();
    map.put(0, now);
    map.put(1, now);
    return new ConfigState(now, map);
  }

  public ConfigState getMainConfigState() throws AdminException {
    long now = System.currentTimeMillis();
    Map<Integer, Long> map = new HashMap<Integer, Long>();
    map.put(0, now);
    map.put(1, now);
    return new ConfigState(now, map);
  }

  public ConfigState getRescheduleConfigState() throws AdminException {
    long now = System.currentTimeMillis();
    Map<Integer, Long> map = new HashMap<Integer, Long>();
    map.put(0, now);
    map.put(1, now);
    return new ConfigState(now, map);
  }

  public ConfigState getClosedGroupConfigState() throws AdminException {
    long now = System.currentTimeMillis();
    Map<Integer, Long> map = new HashMap<Integer, Long>();
    map.put(0, now);
    map.put(1, now);
    return new ConfigState(now, map);
  }

  public ConfigState getFraudConfigState() throws AdminException {
    long now = System.currentTimeMillis();
    Map<Integer, Long> map = new HashMap<Integer, Long>();
    map.put(0, now);
    map.put(1, now);
    return new ConfigState(now, map);
  }

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

  public void addClosedGroup(int groupId, String groupName) throws AdminException {
  }

  public void removeClosedGroup(int groupId) throws AdminException {
  }

  public void addMaskToClosedGroup(int groupId, Address masks) throws AdminException {
  }

  public void removeMaskFromClosedGroup(int groupId, Address masks) throws AdminException {
  }

  public void lockAliases(boolean write) throws AdminException {}

  public void unlockAliases() throws AdminException {}

  public void lockClosedGroups(boolean write) throws AdminException {}

  public void unlockClosedGroups() throws AdminException {}

  public void lockMsc(boolean write) throws AdminException {}

  public void unlockMsc() throws AdminException {}

  public void registerMsc(Address mscAddress) throws AdminException {}

  public void unregisterMsc(Address mscAddress) throws AdminException {}

  public void lockReschedule(boolean write) throws AdminException {}

  public void unlockReschedule() throws AdminException {}

  public void lockFraud(boolean write) throws AdminException {}

  public void unlockFraud() throws AdminException {}

  public void lockMapLimits(boolean write) throws AdminException {}

  public void unlockMapLimits() throws AdminException {}

  public void lockSnmp(boolean write) throws AdminException {}

  public void unlockSnmp() throws AdminException {}

  public void lockMainConfig(boolean write) throws AdminException {}

  public void unlockMainConfig() throws AdminException {}

}

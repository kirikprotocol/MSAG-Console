package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.protocol.ConfigType;
import ru.novosoft.smsc.util.Address;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * @author Artem Snopkov
 */
public class TestClusterControllerStub extends ClusterController {

  private final Lock rescheduleLock = new ReentrantLock();
  private long rescheduleLastUpdateTime = System.currentTimeMillis();

  private final Lock fraudLock = new ReentrantLock();
  private long fraudLastUpdateTime = System.currentTimeMillis();

  private final Lock mapLimitLock = new ReentrantLock();
  private long mapLimitLastUpdateTime = System.currentTimeMillis();

  private final Lock snmpLock = new ReentrantLock();
  private long snmpLastUpdateTime = System.currentTimeMillis();

  private final Lock aliasLock = new ReentrantLock();
  private long aliasLastUpdateTime = System.currentTimeMillis();

  private final Lock closedGroupLock = new ReentrantLock();
  private long closedGroupLastUpdateTime;

  private final Lock mscLock = new ReentrantLock();
  private long mscLastUpdateTime = System.currentTimeMillis();

  private final Lock mainConfigLock = new ReentrantLock();

  private final Lock smeLock = new ReentrantLock();
  private long lastSmeUpdateTime = System.currentTimeMillis();

  private final Lock resourceLock = new ReentrantLock();
  private long lastResourceUpdateTime = System.currentTimeMillis();

  private long aclLastUpdateTime = System.currentTimeMillis();

  private int smscInstancesNumber = 2;

  public TestClusterControllerStub() {
  }

  protected synchronized ConfigState getConfigState(ConfigType configType) throws AdminException {
    long time;
    switch (configType) {
      case Reschedule:
        time = rescheduleLastUpdateTime;
        break;
      case Fraud:
        time = fraudLastUpdateTime;
        break;
      case MapLimits:
        time = mapLimitLastUpdateTime;
        break;
      case Snmp:
        time = snmpLastUpdateTime;
        break;
      case Aliases:
        time = aliasLastUpdateTime;
        break;
      case ClosedGroups:
        time = closedGroupLastUpdateTime;
        break;
      case Msc:
        time = mscLastUpdateTime;
        break;
      case Resources:
        time = lastResourceUpdateTime;
        break;
      case Sme:
        time = lastSmeUpdateTime;
        break;
      default:
        time = System.currentTimeMillis();
    }

    Map<Integer, Long> map = new HashMap<Integer, Long>();
    for (int i = 0; i < smscInstancesNumber; i++)
      map.put(i, time);
    return new ConfigState(time, map);
  }

  public boolean isOnline() {
    return true;
  }

  private void lock(Lock lock, String lockName, boolean write) {
    if (write) {
      System.out.println("Try to lock " + lockName + " for write...");
      lock.lock();
      System.out.println(lockName + " locked for write.");
    } else {
      System.out.println("Try to lock " + lockName + " for read...");
      lock.lock();
      System.out.println(lockName + " locked for read.");
    }
  }

  private void unlock(Lock lock, String lockName) {
    System.out.println("Unlock " + lockName);
    lock.unlock();
  }

  // RESCHEDULE ========================================================================================================

  public void lockReschedule(boolean write) throws AdminException {
    lock(rescheduleLock, "reschedule", write);
  }

  public void unlockReschedule() throws AdminException {
    unlock(rescheduleLock, "reschedule");
  }

  public void applyReschedule() throws AdminException {
    rescheduleLastUpdateTime = System.currentTimeMillis();
  }

  public long getRescheduleLastUpdateTime() {
    return rescheduleLastUpdateTime;
  }

  // FRAUD =============================================================================================================

  public void lockFraud(boolean write) throws AdminException {
    lock(fraudLock, "fraud", write);
  }

  public void unlockFraud() throws AdminException {
    unlock(fraudLock, "fraud");
  }

  public void applyFraud() throws AdminException {
    fraudLastUpdateTime = System.currentTimeMillis();
  }

  public long getFraudLastUpdateTime() {
    return fraudLastUpdateTime;
  }

  // MAP LIMIT =========================================================================================================

  public void lockMapLimits(boolean write) throws AdminException {
    lock(mapLimitLock, "mapLimit", write);
  }

  public void unlockMapLimits() throws AdminException {
    unlock(mapLimitLock, "mapLimit");
  }

  public void applyMapLimits() throws AdminException {
    mapLimitLastUpdateTime = System.currentTimeMillis();
  }

  public long getMapLimitLastUpdateTime() {
    return mapLimitLastUpdateTime;
  }

  // SNMP ==============================================================================================================

  public void lockSnmp(boolean write) throws AdminException {
    lock(snmpLock, "snmp", write);
  }

  public void unlockSnmp() throws AdminException {
    unlock(snmpLock, "snmp");
  }

  public void applySnmp() {
    snmpLastUpdateTime = System.currentTimeMillis();
  }

  public long getSnmpLastUpdateTime() {
    return snmpLastUpdateTime;
  }

  // ALIASES ===========================================================================================================

  public void lockAliases(boolean write) throws AdminException {
    lock(aliasLock, "alias", write);
  }

  public void unlockAliases() throws AdminException {
    unlock(aliasLock, "alias");
  }

  public void addAlias(Address address, Address alias, boolean aliasHide) throws AdminException {
    aliasLastUpdateTime = System.currentTimeMillis();
  }

  public void delAlias(Address alias) throws AdminException {
    aliasLastUpdateTime = System.currentTimeMillis();
  }

  public long getAliasLastUpdateTime() {
    return aliasLastUpdateTime;
  }

  // CLOSED GROUP ======================================================================================================

  public void lockClosedGroups(boolean write) throws AdminException {
    lock(closedGroupLock, "closed group", write);
  }

  public void unlockClosedGroups() throws AdminException {
    unlock(closedGroupLock, "closed group");
  }

  public void addClosedGroup(int groupId, String groupName) throws AdminException {
    closedGroupLastUpdateTime = System.currentTimeMillis();
  }

  public void removeClosedGroup(int groupId) throws AdminException {
    closedGroupLastUpdateTime = System.currentTimeMillis();
  }

  public void addMaskToClosedGroup(int groupId, Address masks) throws AdminException {
    closedGroupLastUpdateTime = System.currentTimeMillis();
  }

  public void removeMaskFromClosedGroup(int groupId, Address masks) throws AdminException {
    closedGroupLastUpdateTime = System.currentTimeMillis();
  }

  public long getClosedGroupLastUpdateTime() {
    return closedGroupLastUpdateTime;
  }

  // MSC ===============================================================================================================

  public void lockMsc(boolean write) throws AdminException {
    lock(mscLock, "msc", write);
  }

  public void unlockMsc() throws AdminException {
    unlock(mscLock, "msc");
  }

  public void registerMsc(Address mscAddress) throws AdminException {
    mscLastUpdateTime = System.currentTimeMillis();
  }

  public void unregisterMsc(Address mscAddress) throws AdminException {
    mscLastUpdateTime = System.currentTimeMillis();
  }

  public long getMscLastUpdateTime() {
    return mscLastUpdateTime;
  }

  // MAIN CONFIG =======================================================================================================

  public void lockMainConfig(boolean write) throws AdminException {
    lock(mainConfigLock, "main config", write);
  }

  public void unlockMainConfig() throws AdminException {
    unlock(mainConfigLock, "main config");
  }

  // SME ===============================================================================================================


  public void lockSmeConfig(boolean write) throws AdminException {
    lock(smeLock, "sme", write);
  }

  public void unlockSmeConfig() throws AdminException {
    unlock(smeLock, "sme");
  }

  public void addSme(CCSme sme) throws AdminException {
    lastSmeUpdateTime = System.currentTimeMillis();
  }

  public void updateSme(CCSme sme) throws AdminException {
    lastSmeUpdateTime = System.currentTimeMillis();
  }

  public void removeSme(String smeId) throws AdminException {
    lastSmeUpdateTime = System.currentTimeMillis();
  }

  public void disconnectSmes(String[] smeIds) throws AdminException {
  }

  public CCSmeSmscStatuses[] getSmesStatus() throws AdminException {
    return new CCSmeSmscStatuses[0];
  }

  // RESOURCES =========================================================================================================

  public void lockResources(boolean write) throws AdminException {
    lock(resourceLock, "resource", write);
  }

  public void unlockResources() throws AdminException {
    unlock(resourceLock, "resource");
  }

  public void applyResources() throws AdminException {
    lastResourceUpdateTime = System.currentTimeMillis();
  }

  // ACCESS CONTROL LIST ===============================================================================================

  public List<CCAclInfo> getAcls() throws AdminException {
    return new ArrayList<CCAclInfo>();
  }

  public List<Address> getAclAddresses(int aclId) throws AdminException {
    return new ArrayList<Address>();
  }

  public void createAlc(int aclId, String name, String description, List<Address> addresses) throws AdminException {
    aclLastUpdateTime = System.currentTimeMillis();
  }

  public void updateAcl(CCAclInfo acl) throws AdminException {
    aclLastUpdateTime = System.currentTimeMillis();
  }

  public void removeAcl(int aclId) throws AdminException {
    aclLastUpdateTime = System.currentTimeMillis();
  }

  public void addAddressesToAcl(int aclId, List<Address> addresses) throws AdminException {
    aclLastUpdateTime = System.currentTimeMillis();
  }

  public void removeAddressesFromAcl(int aclId, List<Address> addresses) throws AdminException {
    aclLastUpdateTime = System.currentTimeMillis();
  }

  public CCAclInfo getAcl(int aclId) throws AdminException {
    return null;
  }
}

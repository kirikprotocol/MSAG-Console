package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.alias.TestAliasManager;
import ru.novosoft.smsc.admin.cluster_controller.protocol.*;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.msc.TestMscManager;
import ru.novosoft.smsc.util.Address;

import java.io.File;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * @author Artem Snopkov
 */
public class TestClusterController extends ClusterController {

  private final TestAclHelper aclHelper = new TestAclHelper();
  private final TestProfilesHelper profilesHelper;

  private final int smscInstancesNumber;
  private final File aliasesFile;
  private final File mscsFile;

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

  private final Lock smeLock = new ReentrantLock();
  private long smeLastUpdateTime = System.currentTimeMillis();

  private final Lock resourceLock = new ReentrantLock();
  private long resourceLastUpdateTime = System.currentTimeMillis();

  private final Lock mainConfigLock = new ReentrantLock();

  private final Lock routesLock = new ReentrantLock();
  private long lastRoutesUpdateTime = System.currentTimeMillis();

  private final Lock timezonesLock = new ReentrantLock();
  private long lastTimezonesUpdateTime = System.currentTimeMillis();

  private final Lock profilesLock = new ReentrantLock();
  private long lastProfilesUpdateTime = System.currentTimeMillis();


  private long aclLastUpdateTime = System.currentTimeMillis();

  public TestClusterController(File aliasesFile, File mscsFile, File profilesFile, boolean smsx, int version,  FileSystem fs, int smscInstancesNumber) throws AdminException {
    this.smscInstancesNumber = smscInstancesNumber;
    this.aliasesFile = aliasesFile;
    this.mscsFile = mscsFile;
    this.profilesHelper = new TestProfilesHelper(profilesFile, fs, smsx, version);
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
      case Sme:
        time = smeLastUpdateTime;
        break;
      case Resources:
        time = resourceLastUpdateTime;
        break;
      case Acl:
        time = aclLastUpdateTime;
        break;
      case Routes:
        time = lastRoutesUpdateTime;
        break;
      case TimeZones:
        time = lastTimezonesUpdateTime;
        break;
      case Profiles:
        time = lastProfilesUpdateTime;
        break;
      default:
        time = System.currentTimeMillis();
    }

    Map<Integer, Long> map = new HashMap<Integer, Long>();
    for (int i = 0; i < smscInstancesNumber; i++) {
      if (i+1 == smscInstancesNumber && configType == ConfigType.Reschedule)
        time = 0;

      map.put(i, time);
    }
    return new ConfigState(time, map);
  }

  public boolean isOnline() {
    return true;
  }

  public void shutdown() {
    
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
    TestAliasManager.helpAddAlias(aliasesFile, address, alias, aliasHide);
    aliasLastUpdateTime = System.currentTimeMillis();
  }

  public void delAlias(Address alias) throws AdminException {
    TestAliasManager.helpRemoveAlias(aliasesFile, alias);
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
    TestMscManager.helpAddMsc(mscsFile, mscAddress);
    mscLastUpdateTime = System.currentTimeMillis();
  }

  public void unregisterMsc(Address mscAddress) throws AdminException {
    TestMscManager.helpRemoveMsc(mscsFile, mscAddress);
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

  /**
   * Разблокирует конфигурацию SME
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unlockSmeConfig() throws AdminException {
    unlock(smeLock, "sme");
  }


  /**
   * Отправляет команду на добавление SME
   * @param sme информация об SME
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void addSme(CCSme sme) throws AdminException {
    smeLastUpdateTime = System.currentTimeMillis();
  }

  /**
   * Отправляет команду на обновление настроек SME
   * @param sme новый настройки SME
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void updateSme(CCSme sme) throws AdminException {
    smeLastUpdateTime = System.currentTimeMillis();
  }

  /**
   * Отправляет команду на удаление SME
   * @param smeId идентификатор SME
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void removeSme(String smeId) throws AdminException {
    smeLastUpdateTime = System.currentTimeMillis();
  }

  /**
   * Отправляет команду на отключение одной или нескольких SME от центра
   * @param smeIds идентификаторы SME-х. которых надо отключить
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
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
    resourceLastUpdateTime = System.currentTimeMillis();
  }

  // ACCESS CONTROL LIST ===============================================================================================

  public List<CCAclInfo> getAcls() throws AdminException {
    return aclHelper.getAcls();
  }

  public List<Address> getAclAddresses(int aclId) throws AdminException {
    return aclHelper.getAclAddresses(aclId);
  }

  public void createAlc(int aclId, String name, String description, List<Address> addresses) throws AdminException {
    aclLastUpdateTime = System.currentTimeMillis();
    aclHelper.createAcl(aclId, name, description, addresses);
  }

  public void updateAcl(CCAclInfo acl) throws AdminException {
    aclLastUpdateTime = System.currentTimeMillis();
    aclHelper.updateAcl(acl);
  }

  public void removeAcl(int aclId) throws AdminException {
    aclLastUpdateTime = System.currentTimeMillis();
    aclHelper.removeAcl(aclId);
  }

  public void addAddressesToAcl(int aclId, List<Address> addresses) throws AdminException {
    aclLastUpdateTime = System.currentTimeMillis();
    aclHelper.addAddresses(aclId, addresses);
  }

  public void removeAddressesFromAcl(int aclId, List<Address> addresses) throws AdminException {
    aclLastUpdateTime = System.currentTimeMillis();
    aclHelper.removeAddresses(aclId, addresses);
  }

  public CCAclInfo getAcl(int aclId) throws AdminException {
    return aclHelper.getAcl(aclId);
  }

  // ROUTES ============================================================================================================

  public void applyRoutes() throws AdminException {
    lastRoutesUpdateTime = System.currentTimeMillis();
  }

  public void unlockRoutes() throws AdminException {
    unlock(routesLock, "routes");
  }

  public void lockRoutes(boolean write) throws AdminException {
    lock(routesLock, "routes", write);
  }

  public CCRouteTrace traceRoute(String fileName, Address source, Address destination, String sourceSmeId) throws AdminException {
    return new CCRouteTrace(null, new String[0]);
  }

  // TIMEZONES =========================================================================================================

  @Override
  public void lockTimezones(boolean write) throws AdminException {
    lock(timezonesLock, "timezones", write);
  }

  @Override
  public void unlockTimezones() throws AdminException {
    unlock(timezonesLock, "timezones");
  }

  @Override
  public void applyTimezones() throws AdminException {
    lastTimezonesUpdateTime = System.currentTimeMillis();
  }

  // PROFILES ================================================================================================

  public void lockProfiles(boolean write) throws AdminException {
    lock(profilesLock, "profiles", write);
  }

  public void unlockProfiles() throws AdminException {
    unlock(profilesLock, "profiles");
  }

  public CCLookupProfileResult lookupProfile(Address address) throws AdminException {
    return profilesHelper.lookupProfile(address);
  }

  public void updateProfile(Address address, CCProfile profile) throws AdminException {
    profilesHelper.updateProfile(address, profile);
    lastProfilesUpdateTime = System.currentTimeMillis();
  }
  
  public void deleteProfile(Address address) throws AdminException {
    profilesHelper.deleteProfile(address);
    lastProfilesUpdateTime = System.currentTimeMillis();
  }
}

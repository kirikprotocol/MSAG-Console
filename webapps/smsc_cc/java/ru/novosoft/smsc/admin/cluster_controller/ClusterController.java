package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.protocol.*;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.ServiceManager;
import ru.novosoft.smsc.util.Address;

import java.util.HashMap;
import java.util.Map;

/**
 * Класс для отправки комманд в ClusterController.
 *
 * @author Artem Snopkov
 */
public class ClusterController {

  public static final String SERVICE_ID = "ClusterController";


  private ClusterControllerClient cc;
  private ServiceManager serviceManager;
  private long lastConfigsStatusCheckTime;
  private GetConfigsStateResp lastGetConfigsStateResp;

  protected ClusterController() {

  }

  public ClusterController(ClusterControllerManager manager, ServiceManager serviceManager) throws AdminException {
    this.serviceManager = serviceManager;

    this.cc = new ClusterControllerClient(manager, serviceManager);
  }

  private ServiceInfo getInfo() throws AdminException {
    ServiceInfo si = serviceManager.getService(SERVICE_ID);
    if (si == null)
      throw new ClusterControllerException("cluster_controller_offline");
    return si;
  }

  public boolean isOnline() throws AdminException {
    return getInfo().getOnlineHost() != null;
  }

  public void shutdown() {
    cc.shutdown();
  }

  private synchronized ConfigState getConfigState(ConfigType configType) throws AdminException {
    long now = System.currentTimeMillis();

    if (now - lastConfigsStatusCheckTime > 1000 || lastGetConfigsStateResp == null) {
      GetConfigsState req = new GetConfigsState();
      lastGetConfigsStateResp = cc.send(req);
      lastConfigsStatusCheckTime = now;
    }

    long ccUpdateTime = lastGetConfigsStateResp.getCcConfigUpdateTime()[configType.getValue()];

    SmscConfigsState[] states = lastGetConfigsStateResp.getSmscConfigs();
    Map<Integer, Long> instancesUpdateTimes = new HashMap<Integer, Long>();
    for (SmscConfigsState state : states)
      instancesUpdateTimes.put((int)state.getNodeIdex(), state.getUpdateTime()[configType.getValue()]);

    return new ConfigState(ccUpdateTime, instancesUpdateTimes);
  }

  // GLOBAL ============================================================================================================

  /**
   * Блокирует главный конфиг СМСЦ для чтения/записи
   *
   * @param write блокировать файл для записи
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void lockMainConfig(boolean write) throws AdminException {
    LockConfig req = new LockConfig();
    req.setConfigType(ConfigType.MainConfig);
    req.setWriteLock(write);
    LockConfigResp resp = cc.send(req);
    if (resp.getResp().getStatus() != 0)
      throw new ClusterControllerException("interaction_error", resp.getResp().getStatus() + "");
  }

  /**
   * Разблокирует главный конфиг СМСЦ
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unlockMainConfig() throws AdminException {
    UnlockConfig req = new UnlockConfig();
    req.setConfigType(ConfigType.MainConfig);
    cc.send(req);
  }

  /**
   * Возвращает статус главного конфига
   * @return статус главного конфига
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public ConfigState getMainConfigState() throws AdminException {
    return getConfigState(ConfigType.MainConfig);
  }

  // ALIASES ===========================================================================================================

  /**
   * Блокирует конфигурацию алиасов для чтения/записи
   *
   * @param write блокировать файл для записи
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void lockAliases(boolean write) throws AdminException {
    LockConfig req = new LockConfig();
    req.setConfigType(ConfigType.Aliases);
    req.setWriteLock(write);
    LockConfigResp resp = cc.send(req);
    if (resp.getResp().getStatus() != 0)
      throw new ClusterControllerException("interaction_error", resp.getResp().getStatus() + "");
  }

  /**
   * Разблокирует конфигурацию алиасов
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unlockAliases() throws AdminException {
    UnlockConfig req = new UnlockConfig();
    req.setConfigType(ConfigType.Aliases);
    cc.send(req);
  }

  /**
   * Отправляет команду на добавления алиаса
   *
   * @param address   адрес
   * @param alias     алиас
   * @param aliasHide признак скрытности алиаса
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void addAlias(String address, String alias, boolean aliasHide) throws AdminException {
    AliasAdd req = new AliasAdd();
    req.setAddr(address);
    req.setAlias(alias);
    req.setHide(aliasHide);
    MultiResponse resp = cc.send(req).getResp();
    int[] statuses = resp.getStatus();
    for (int status : statuses) {
      if (status != 0)
        throw new ClusterControllerException("interaction_error", statuses, resp.getIds());
    }
  }

  /**
   * Отправляет команду на удаление алиаса
   *
   * @param alias алиас
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void delAlias(String alias) throws AdminException {
    AliasDel req = new AliasDel();
    req.setAlias(alias);
    MultiResponse resp = cc.send(req).getResp();
    int[] statuses = resp.getStatus();
    for (int status : statuses) {
      if (status != 0)
        throw new ClusterControllerException("interaction_error", statuses, resp.getIds());
    }
  }

  /**
   * Возвращает статус конфига алиасов
   * @return статус конфига алиасов
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public ConfigState getAliasesConfigState() throws AdminException {
    return getConfigState(ConfigType.Aliases);
  }

  // CLOSED GROUPS =====================================================================================================

  /**
   * Блокирует конфигурацию закрытых групп для чтения/записи
   *
   * @param write блокировать файл для записи
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void lockClosedGroups(boolean write) throws AdminException {
    LockConfig req = new LockConfig();
    req.setConfigType(ConfigType.ClosedGroups);
    req.setWriteLock(write);
    LockConfigResp resp = cc.send(req);
    if (resp.getResp().getStatus() != 0)
      throw new ClusterControllerException("interaction_error", resp.getResp().getStatus() + "");
  }

  /**
   * Разблокирует конфигурацию закрытых групп
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unlockClosedGroups() throws AdminException {
    UnlockConfig req = new UnlockConfig();
    req.setConfigType(ConfigType.ClosedGroups);
    cc.send(req);
  }

  /**
   * Отправляет команду на добавление закрытой группы
   *
   * @param groupId   идентификатор группы
   * @param groupName название группы
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void addClosedGroup(int groupId, String groupName) throws AdminException {
    CgmAddGroup req = new CgmAddGroup();
    req.setId(groupId);
    req.setName(groupName);
    MultiResponse resp = cc.send(req).getResp();
    int[] statuses = resp.getStatus();
    for (int status : statuses) {
      if (status != 0)
        throw new ClusterControllerException("interaction_error", statuses, resp.getIds());
    }
  }

  /**
   * Отправляет команду на удаление закрытой группы
   *
   * @param groupId идентификатор группы
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void removeClosedGroup(int groupId) throws AdminException {
    CgmDeleteGroup req = new CgmDeleteGroup();
    req.setId(groupId);
    MultiResponse resp = cc.send(req).getResp();
    int[] statuses = resp.getStatus();
    for (int status : statuses) {
      if (status != 0)
        throw new ClusterControllerException("interaction_error", statuses, resp.getIds());
    }
  }

  /**
   * Отправляет команду на добавление маски в закрытую группу
   *
   * @param groupId идентификатор группы
   * @param mask    маска для добавления
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void addMaskToClosedGroup(int groupId, Address mask) throws AdminException {
    CgmAddAddr req = new CgmAddAddr();
    req.setId(groupId);
    req.setAddr(mask.getNormalizedAddress());
    MultiResponse resp = cc.send(req).getResp();
    int[] statuses = resp.getStatus();
    for (int status : statuses) {
      if (status != 0)
        throw new ClusterControllerException("interaction_error", statuses, resp.getIds());
    }
  }

  /**
   * Отправляет команду на удаление маски из закрытой группы
   *
   * @param groupId идентификатор группы
   * @param mask    маска для удаления
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void removeMaskFromClosedGroup(int groupId, Address mask) throws AdminException {
    CgmDelAddr req = new CgmDelAddr();
    req.setId(groupId);
    req.setAddr(mask.getNormalizedAddress());
    MultiResponse resp = cc.send(req).getResp();
    int[] statuses = resp.getStatus();
    for (int status : statuses) {
      if (status != 0)
        throw new ClusterControllerException("interaction_error", statuses, resp.getIds());
    }
  }

  // MSC ===============================================================================================================

  /**
   * Блокирует конфигурацию MSC для чтения/записи
   *
   * @param write блокировать файл для записи
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void lockMsc(boolean write) throws AdminException {
    LockConfig req = new LockConfig();
    req.setConfigType(ConfigType.Msc);
    req.setWriteLock(write);
    LockConfigResp resp = cc.send(req);
    if (resp.getResp().getStatus() != 0)
      throw new ClusterControllerException("interaction_error", resp.getResp().getStatus() + "");
  }

  /**
   * Разблокирует конфигурацию Msc
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unlockMsc() throws AdminException {
    UnlockConfig req = new UnlockConfig();
    req.setConfigType(ConfigType.Msc);
    cc.send(req);
  }

  /**
   * Регистрирует новый MSC адрес в СС
   *
   * @param mscAddress новый MSC адрес
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void registerMsc(Address mscAddress) throws AdminException {
    MscAdd req = new MscAdd();
    req.setMsc(mscAddress.getNormalizedAddress());
    MultiResponse resp = cc.send(req).getResp();
    int[] statuses = resp.getStatus();
    for (int status : statuses) {
      if (status != 0)
        throw new ClusterControllerException("interaction_error", statuses, resp.getIds());
    }
  }

  /**
   * Удаляет MSC адрес из реестра СС
   *
   * @param mscAddress MSC адрес, который надо удалить из реестра
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unregisterMsc(Address mscAddress) throws AdminException {
    MscRemove req = new MscRemove();
    req.setMsc(mscAddress.getNormalizedAddress());
    MultiResponse resp = cc.send(req).getResp();
    int[] statuses = resp.getStatus();
    for (int status : statuses) {
      if (status != 0)
        throw new ClusterControllerException("interaction_error", statuses, resp.getIds());
    }
  }

  // RESCHEDULE ========================================================================================================

  /**
   * Блокирует конфигурацию политик передоставки для чтения/записи
   *
   * @param write блокировать файл для записи
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void lockReschedule(boolean write) throws AdminException {
    LockConfig req = new LockConfig();
    req.setConfigType(ConfigType.Reschedule);
    req.setWriteLock(write);
    LockConfigResp resp = cc.send(req);
    if (resp.getResp().getStatus() != 0)
      throw new ClusterControllerException("interaction_error", resp.getResp().getStatus() + "");
  }

  /**
   * Разблокирует конфигурацию политик передоставки
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unlockReschedule() throws AdminException {
    UnlockConfig req = new UnlockConfig();
    req.setConfigType(ConfigType.Reschedule);
    cc.send(req);
  }

  /**
   * Отправляет команду на применение изменений в конфигурации политик передоставки
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void applyReschedule() throws AdminException {
    MultiResponse resp = cc.send(new ApplyReschedule()).getResp();
    int[] statuses = resp.getStatus();
    for (int status : statuses) {
      if (status != 0)
        throw new ClusterControllerException("interaction_error", statuses, resp.getIds());
    }
  }

  /**
   * Возвращает статус конфига политик передоставки
   * @return статус конфига политик передоставки
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public ConfigState getRescheduleConfigState() throws AdminException {
    return getConfigState(ConfigType.Reschedule);
  }

  // FRAUD =============================================================================================================

  /**
   * Блокирует конфигурацию fraud для чтения/записи
   *
   * @param write блокировать файл для записи
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void lockFraud(boolean write) throws AdminException {
    LockConfig req = new LockConfig();
    req.setConfigType(ConfigType.Fraud);
    req.setWriteLock(write);
    LockConfigResp resp = cc.send(req);
    if (resp.getResp().getStatus() != 0)
      throw new ClusterControllerException("interaction_error", resp.getResp().getStatus() + "");
  }

  /**
   * Разблокирует конфигурацию fraud
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unlockFraud() throws AdminException {
    UnlockConfig req = new UnlockConfig();
    req.setConfigType(ConfigType.Fraud);
    cc.send(req);
  }

  /**
   * Отправляет команду на применение изменений в конфигурации fraud
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void applyFraud() throws AdminException {
    MultiResponse resp = cc.send(new ApplyFraudControl()).getResp();
    int[] statuses = resp.getStatus();
    for (int status : statuses) {
      if (status != 0)
        throw new ClusterControllerException("interaction_error", statuses, resp.getIds());
    }
  }

  // MAP LIMITS ========================================================================================================

  /**
   * Блокирует конфигурацию Map Limits для чтения/записи
   *
   * @param write блокировать файл для записи
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void lockMapLimits(boolean write) throws AdminException {
    LockConfig req = new LockConfig();
    req.setConfigType(ConfigType.MapLimits);
    req.setWriteLock(write);
    LockConfigResp resp = cc.send(req);
    if (resp.getResp().getStatus() != 0)
      throw new ClusterControllerException("interaction_error", resp.getResp().getStatus() + "");
  }

  /**
   * Разблокирует конфигурацию Map Limits
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unlockMapLimits() throws AdminException {
    UnlockConfig req = new UnlockConfig();
    req.setConfigType(ConfigType.MapLimits);
    cc.send(req);
  }

  /**
   * Отправляет команду на применение изменений в Map Limits
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void applyMapLimits() throws AdminException {
    MultiResponse resp = cc.send(new ApplyMapLimits()).getResp();
    int[] statuses = resp.getStatus();
    for (int status : statuses) {
      if (status != 0)
        throw new ClusterControllerException("interaction_error", statuses, resp.getIds());
    }
  }

  // SNMP ==============================================================================================================

  /**
   * Блокирует конфигурацию SNMP для чтения/записи
   *
   * @param write блокировать файл для записи
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void lockSnmp(boolean write) throws AdminException {
    LockConfig req = new LockConfig();
    req.setConfigType(ConfigType.Snmp);
    req.setWriteLock(write);
    LockConfigResp resp = cc.send(req);
    if (resp.getResp().getStatus() != 0)
      throw new ClusterControllerException("interaction_error", resp.getResp().getStatus() + "");
  }

  /**
   * Разблокирует конфигурацию SNMP
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unlockSnmp() throws AdminException {
    UnlockConfig req = new UnlockConfig();
    req.setConfigType(ConfigType.Snmp);
    cc.send(req);
  }

  /**
   * Отправляет команду на применение изменений в конфиге SNMP
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void applySnmp() throws AdminException {
    MultiResponse resp = cc.send(new ApplySnmp()).getResp();
    int[] statuses = resp.getStatus();
    for (int status : statuses) {
      if (status != 0)
        throw new ClusterControllerException("interaction_error", statuses, resp.getIds());
    }
  }
}

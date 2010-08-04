package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.protocol.*;
import ru.novosoft.smsc.util.Address;

import java.util.EnumMap;
import java.util.HashMap;
import java.util.Map;

/**
 * Класс для отправки комманд в ClusterController.
 *
 * @author Artem Snopkov
 */
public class ClusterController {




  private ClusterControllerClient cc;
  private long lastConfigsStatusCheckTime;
  private EnumMap<ConfigType, ConfigState> configsStates;

  protected ClusterController() {

  }

  public ClusterController(ClusterControllerManager manager) throws AdminException {
    this.cc = new ClusterControllerClient(manager);
  }

  public boolean isOnline() throws AdminException {
    return cc.isConnected();
  }

  public void shutdown() {
    cc.shutdown();
  }

  protected synchronized void lockConfig(ConfigType configType, boolean write) throws AdminException {
    LockConfig req = new LockConfig();
    req.setConfigType(configType);
    req.setWriteLock(write);
    LockConfigResp resp = cc.send(req);
    if (resp.getResp().getStatus() != 0)
      throw new ClusterControllerException("interaction_error", resp.getResp().getStatus() + "");
  }

  protected synchronized void unlockConfig(ConfigType configType) throws AdminException {
    UnlockConfig req = new UnlockConfig();
    req.setConfigType(configType);
    cc.send(req);
  }

  protected synchronized ConfigState getConfigState(ConfigType configType) throws AdminException {
    long now = System.currentTimeMillis();

    if (now - lastConfigsStatusCheckTime > 1000 || configsStates == null) {
      GetConfigsState req = new GetConfigsState();
      GetConfigsStateResp resp = cc.send(req);

      if (configsStates == null)
        configsStates = new EnumMap<ConfigType, ConfigState>(ConfigType.class);
      else
        configsStates.clear();

      for (ConfigType type : ConfigType.values()) {
        long ccUpdateTime = resp.getCcConfigUpdateTime()[type.getValue()];

        SmscConfigsState[] states = resp.getSmscConfigs();
        Map<Integer, Long> instancesUpdateTimes = new HashMap<Integer, Long>();
        for (SmscConfigsState state : states)
          instancesUpdateTimes.put((int)state.getNodeIdex(), state.getUpdateTime()[type.getValue()]);

        ConfigState state = new ConfigState(ccUpdateTime, instancesUpdateTimes);

        configsStates.put(type, state);
      }
      lastConfigsStatusCheckTime = now;
    }

    return configsStates.get(configType);
  }

  // GLOBAL ============================================================================================================

  /**
   * Блокирует главный конфиг СМСЦ для чтения/записи
   *
   * @param write блокировать файл для записи
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void lockMainConfig(boolean write) throws AdminException {
    lockConfig(ConfigType.MainConfig, write);
  }

  /**
   * Разблокирует главный конфиг СМСЦ
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unlockMainConfig() throws AdminException {
    unlockConfig(ConfigType.MainConfig);
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
    lockConfig(ConfigType.Aliases, write);
  }

  /**
   * Разблокирует конфигурацию алиасов
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unlockAliases() throws AdminException {
    unlockConfig(ConfigType.Aliases);
  }

  /**
   * Отправляет команду на добавления алиаса
   *
   * @param address   адрес
   * @param alias     алиас
   * @param aliasHide признак скрытности алиаса
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void addAlias(Address address, Address alias, boolean aliasHide) throws AdminException {
    lastConfigsStatusCheckTime = 0;
    AliasAdd req = new AliasAdd();
    req.setAddr(address.getNormalizedAddress());
    req.setAlias(alias.getNormalizedAddress());
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
  public void delAlias(Address alias) throws AdminException {
    lastConfigsStatusCheckTime = 0;
    AliasDel req = new AliasDel();
    req.setAlias(alias.getNormalizedAddress());
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
    lockConfig(ConfigType.ClosedGroups, write);
  }

  /**
   * Разблокирует конфигурацию закрытых групп
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unlockClosedGroups() throws AdminException {
   unlockConfig(ConfigType.ClosedGroups);
  }

  /**
   * Отправляет команду на добавление закрытой группы
   *
   * @param groupId   идентификатор группы
   * @param groupName название группы
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void addClosedGroup(int groupId, String groupName) throws AdminException {
    lastConfigsStatusCheckTime = 0;
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
    lastConfigsStatusCheckTime = 0;
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
    lastConfigsStatusCheckTime = 0;
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
    lastConfigsStatusCheckTime = 0;
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

  /**
   * Возвращает статус конфигу закрытых групп
   * @return статус конфигу закрытых групп
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public ConfigState getClosedGroupConfigState() throws AdminException {
    return getConfigState(ConfigType.ClosedGroups);
  }

  // MSC ===============================================================================================================

  /**
   * Блокирует конфигурацию MSC для чтения/записи
   *
   * @param write блокировать файл для записи
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void lockMsc(boolean write) throws AdminException {
    lockConfig(ConfigType.Msc, write);
  }

  /**
   * Разблокирует конфигурацию Msc
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unlockMsc() throws AdminException {
    unlockConfig(ConfigType.Msc);
  }

  /**
   * Регистрирует новый MSC адрес в СС
   *
   * @param mscAddress новый MSC адрес
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void registerMsc(Address mscAddress) throws AdminException {
    lastConfigsStatusCheckTime = 0;
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
    lastConfigsStatusCheckTime = 0;
    MscRemove req = new MscRemove();
    req.setMsc(mscAddress.getNormalizedAddress());
    MultiResponse resp = cc.send(req).getResp();
    int[] statuses = resp.getStatus();
    for (int status : statuses) {
      if (status != 0)
        throw new ClusterControllerException("interaction_error", statuses, resp.getIds());
    }
  }

  /**
   * Возвращает статус конфига Msc
   * @return статус конфига Msc
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public ConfigState getMscConfigState() throws AdminException {
    return getConfigState(ConfigType.Msc);
  }

  // RESCHEDULE ========================================================================================================

  /**
   * Блокирует конфигурацию политик передоставки для чтения/записи
   *
   * @param write блокировать файл для записи
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void lockReschedule(boolean write) throws AdminException {
    lockConfig(ConfigType.Reschedule, write);
  }

  /**
   * Разблокирует конфигурацию политик передоставки
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unlockReschedule() throws AdminException {
    unlockConfig(ConfigType.Reschedule);
  }

  /**
   * Отправляет команду на применение изменений в конфигурации политик передоставки
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void applyReschedule() throws AdminException {
    lastConfigsStatusCheckTime = 0;
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
    lockConfig(ConfigType.Fraud, write);
  }

  /**
   * Разблокирует конфигурацию fraud
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unlockFraud() throws AdminException {
    unlockConfig(ConfigType.Fraud);
  }

  /**
   * Отправляет команду на применение изменений в конфигурации fraud
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void applyFraud() throws AdminException {
    lastConfigsStatusCheckTime = 0;
    MultiResponse resp = cc.send(new ApplyFraudControl()).getResp();
    int[] statuses = resp.getStatus();
    for (int status : statuses) {
      if (status != 0)
        throw new ClusterControllerException("interaction_error", statuses, resp.getIds());
    }
  }

  /**
   * Возвращает статус конфига Fraud
   * @return статус конфига Fraud
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public ConfigState getFraudConfigState() throws AdminException {
    return getConfigState(ConfigType.Fraud);
  }

  // MAP LIMITS ========================================================================================================

  /**
   * Блокирует конфигурацию Map Limits для чтения/записи
   *
   * @param write блокировать файл для записи
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void lockMapLimits(boolean write) throws AdminException {
    lockConfig(ConfigType.MapLimits, write);
  }

  /**
   * Разблокирует конфигурацию Map Limits
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unlockMapLimits() throws AdminException {
    unlockConfig(ConfigType.MapLimits);
  }

  /**
   * Отправляет команду на применение изменений в Map Limits
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void applyMapLimits() throws AdminException {
    lastConfigsStatusCheckTime = 0;
    MultiResponse resp = cc.send(new ApplyMapLimits()).getResp();
    int[] statuses = resp.getStatus();
    for (int status : statuses) {
      if (status != 0)
        throw new ClusterControllerException("interaction_error", statuses, resp.getIds());
    }
  }

  /**
   * Возвращает статус конфига Map Limits
   * @return статус конфига Map Limits
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public ConfigState getMapLimitConfigState() throws AdminException {
    return getConfigState(ConfigType.MapLimits);
  }

  // SNMP ==============================================================================================================

  /**
   * Блокирует конфигурацию SNMP для чтения/записи
   *
   * @param write блокировать файл для записи
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void lockSnmp(boolean write) throws AdminException {
    lockConfig(ConfigType.Snmp, write);
  }

  /**
   * Разблокирует конфигурацию SNMP
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unlockSnmp() throws AdminException {
    unlockConfig(ConfigType.Snmp);
  }

  /**
   * Отправляет команду на применение изменений в конфиге SNMP
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void applySnmp() throws AdminException {
    lastConfigsStatusCheckTime = 0;
    MultiResponse resp = cc.send(new ApplySnmp()).getResp();
    int[] statuses = resp.getStatus();
    for (int status : statuses) {
      if (status != 0)
        throw new ClusterControllerException("interaction_error", statuses, resp.getIds());
    }
  }

  /**
   * Возвращает статус конфигурации SNMP
   * @return статус конфигурации SNMP
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public ConfigState getSnmpConfigState() throws AdminException {
    return getConfigState(ConfigType.Snmp);
  }

  // SME ===============================================================================================================

  /**
   * Блокирует конфигурацию SME для чтения/записи
   *
   * @param write блокировать файл для записи
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void lockSmeConfig(boolean write) throws AdminException {
    lockConfig(ConfigType.Sme, write);
  }

  /**
   * Разблокирует конфигурацию SME
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unlockSmeConfig() throws AdminException {
    unlockConfig(ConfigType.Sme);
  }

  /**
   * Возвращает статус конфигурации SME
   * @return статус конфигурации SME
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public ConfigState getSmeConfigState() throws AdminException {
    return getConfigState(ConfigType.Sme);
  }

  /**
   * Отправляет команду на добавление SME
   * @param sme информация об SME
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void addSme(CCSme sme) throws AdminException {
    lastConfigsStatusCheckTime = 0;
    SmeAdd req = new SmeAdd();
    req.setParams(sme.toSmeParams());
    MultiResponse resp = cc.send(req).getResp();
    int[] statuses = resp.getStatus();
    for (int status : statuses) {
      if (status != 0)
        throw new ClusterControllerException("interaction_error", statuses, resp.getIds());
    }
  }

  /**
   * Отправляет команду на обновление настроек SME
   * @param sme новый настройки SME
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void updateSme(CCSme sme) throws AdminException {
    lastConfigsStatusCheckTime = 0;
    SmeUpdate req = new SmeUpdate();
    req.setParams(sme.toSmeParams());
    MultiResponse resp = cc.send(req).getResp();
    int[] statuses = resp.getStatus();
    for (int status : statuses) {
      if (status != 0)
        throw new ClusterControllerException("interaction_error", statuses, resp.getIds());
    }
  }

  /**
   * Отправляет команду на удаление SME
   * @param smeId идентификатор SME
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void removeSme(String smeId) throws AdminException {
    lastConfigsStatusCheckTime = 0;
    SmeRemove req = new SmeRemove();
    req.setSmeId(smeId);
    MultiResponse resp = cc.send(req).getResp();
    int[] statuses = resp.getStatus();
    for (int status : statuses) {
      if (status != 0)
        throw new ClusterControllerException("interaction_error", statuses, resp.getIds());
    }
  }

  /**
   * Отправляет команду на отключение одной или нескольких SME от центра
   * @param smeIds идентификаторы SME-х. которых надо отключить
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void disconnectSmes(String[] smeIds) throws AdminException {
    lastConfigsStatusCheckTime = 0;
    SmeDisconnect req = new SmeDisconnect();
    req.setSysIds(smeIds);
    MultiResponse resp = cc.send(req).getResp();
    int[] statuses = resp.getStatus();
    for (int status : statuses) {
      if (status != 0)
        throw new ClusterControllerException("interaction_error", statuses, resp.getIds());
    }
  }


  public CCSmeSmscStatuses[] getSmesStatus() throws AdminException {
    lastConfigsStatusCheckTime = 0;
    SmeStatus req = new SmeStatus();
    SmeStatusResp resp = cc.send(req);
    if (resp.getResp().getStatus() != 0)
      throw new ClusterControllerException("interaction_error", resp.getResp().getStatus() + "");

    CCSmeSmscStatuses res[] = new CCSmeSmscStatuses[resp.getStatus().length];
    SmeStatusInfo[] statusInfo = resp.getStatus();
    for (int i=0; i<statusInfo.length; i++) {
      SmeStatusInfo smeStatusInfo = statusInfo[i];
      CCSmeSmscStatuses stateInfo = new CCSmeSmscStatuses(smeStatusInfo.getSystemId(), smeStatusInfo.getConnType().getValue());
      for (SmeConnectStatus smeConnectStatus : smeStatusInfo.getStatus()) {
        stateInfo.addConnectStatus(smeConnectStatus.getNodeIdx(),
            smeConnectStatus.getStatus().getValue(),
            smeConnectStatus.getBindMode().getValue(),
            smeConnectStatus.getPeerIn(),
            smeConnectStatus.getPeerOut());
      }
      res[i] = stateInfo;
    }
    return res;
  }

  // RESOURCES =========================================================================================================

  /**
   * Блокирует конфигурацию ресурсов для чтения/записи
   *
   * @param write блокировать файл для записи
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void lockResources(boolean write) throws AdminException {
    lockConfig(ConfigType.Resources, write);
  }

  /**
   * Разблокирует конфигурацию ресурсов
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unlockResources() throws AdminException {
    unlockConfig(ConfigType.Resources);
  }

  /**
   * Отправляет запрос на обновление конфигурации ресурсов
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void applyResources() throws AdminException {
    ApplyLocaleResource req = new ApplyLocaleResource();
    MultiResponse resp = cc.send(req).getResp();
    int[] statuses = resp.getStatus();
    for (int status : statuses) {
      if (status != 0)
        throw new ClusterControllerException("interaction_error", statuses, resp.getIds());
    }
  }

  /**
   * Возвращает статус конфигурации ресурсов
   * @return статус конфигурации ресурсов
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public ConfigState getResourcesState() throws AdminException {
    return getConfigState(ConfigType.Resources);
  }
}

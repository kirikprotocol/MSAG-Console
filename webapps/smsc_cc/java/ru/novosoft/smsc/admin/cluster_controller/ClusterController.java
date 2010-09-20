package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.protocol.*;
import ru.novosoft.smsc.util.Address;

import java.util.*;

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

  protected void checkResponse(MultiResponse resp) throws ClusterControllerException {
    int[] statuses = resp.getStatus();
    for (int status : statuses) {
      if (status != 0)
        throw new ClusterControllerException("interaction_error", statuses, resp.getIds());
    }
  }

  protected void checkResponse(Response resp) throws ClusterControllerException {
    if (resp.getStatus() != 0)
      throw new ClusterControllerException("interaction_error", resp.getStatus() + "");
  }

  protected synchronized void lockConfig(ConfigType configType, boolean write) throws AdminException {
    LockConfig req = new LockConfig();
    req.setConfigType(configType);
    req.setWriteLock(write);
    checkResponse(cc.send(req).getResp());
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

      checkResponse(resp.getResp());

      if (configsStates == null)
        configsStates = new EnumMap<ConfigType, ConfigState>(ConfigType.class);
      else
        configsStates.clear();

      for (ConfigType type : ConfigType.values()) {
        long ccUpdateTime = resp.getCcConfigUpdateTime()[type.getValue()];

        if (resp.hasSmscConfigs()) {
          SmscConfigsState[] states = resp.getSmscConfigs();
          Map<Integer, Long> instancesUpdateTimes = new HashMap<Integer, Long>();
          for (SmscConfigsState state : states)
            instancesUpdateTimes.put((int) state.getNodeIdex()-1, state.getUpdateTime()[type.getValue()]*1000);

          ConfigState state = new ConfigState(ccUpdateTime, instancesUpdateTimes);

          configsStates.put(type, state);
        }
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
   *
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
    checkResponse(cc.send(req).getResp());
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
    checkResponse(cc.send(req).getResp());
  }

  /**
   * Возвращает статус конфига алиасов
   *
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
    checkResponse(cc.send(req).getResp());
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
    checkResponse(cc.send(req).getResp());
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
    checkResponse(cc.send(req).getResp());
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
    checkResponse(cc.send(req).getResp());
  }

  /**
   * Возвращает статус конфигу закрытых групп
   *
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
    checkResponse(cc.send(req).getResp());
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
    checkResponse(cc.send(req).getResp());
  }

  /**
   * Возвращает статус конфига Msc
   *
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
    checkResponse(cc.send(new ApplyReschedule()).getResp());
  }

  /**
   * Возвращает статус конфига политик передоставки
   *
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
    checkResponse(cc.send(new ApplyFraudControl()).getResp());
  }

  /**
   * Возвращает статус конфига Fraud
   *
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
    checkResponse(cc.send(new ApplyMapLimits()).getResp());
  }

  /**
   * Возвращает статус конфига Map Limits
   *
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
    checkResponse(cc.send(new ApplySnmp()).getResp());
  }

  /**
   * Возвращает статус конфигурации SNMP
   *
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
   *
   * @return статус конфигурации SME
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public ConfigState getSmeConfigState() throws AdminException {
    return getConfigState(ConfigType.Sme);
  }

  /**
   * Отправляет команду на добавление SME
   *
   * @param sme информация об SME
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void addSme(CCSme sme) throws AdminException {
    lastConfigsStatusCheckTime = 0;
    SmeAdd req = new SmeAdd();
    req.setParams(sme.toSmeParams());
    checkResponse(cc.send(req).getResp());
  }

  /**
   * Отправляет команду на обновление настроек SME
   *
   * @param sme новый настройки SME
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void updateSme(CCSme sme) throws AdminException {
    lastConfigsStatusCheckTime = 0;
    SmeUpdate req = new SmeUpdate();
    req.setParams(sme.toSmeParams());
    checkResponse(cc.send(req).getResp());
  }

  /**
   * Отправляет команду на удаление SME
   *
   * @param smeId идентификатор SME
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void removeSme(String smeId) throws AdminException {
    lastConfigsStatusCheckTime = 0;
    SmeRemove req = new SmeRemove();
    req.setSmeId(smeId);
    checkResponse(cc.send(req).getResp());
  }

  /**
   * Отправляет команду на отключение одной или нескольких SME от центра
   *
   * @param smeIds идентификаторы SME-х. которых надо отключить
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void disconnectSmes(String[] smeIds) throws AdminException {
    lastConfigsStatusCheckTime = 0;
    SmeDisconnect req = new SmeDisconnect();
    req.setSysIds(smeIds);
    checkResponse(cc.send(req).getResp());
  }


  public CCSmeSmscStatuses[] getSmesStatus() throws AdminException {
    lastConfigsStatusCheckTime = 0;
    SmeStatus req = new SmeStatus();
    SmeStatusResp resp = cc.send(req);
    checkResponse(resp.getResp());

    CCSmeSmscStatuses res[] = new CCSmeSmscStatuses[resp.getStatus().length];
    SmeStatusInfo[] statusInfo = resp.getStatus();
    for (int i = 0; i < statusInfo.length; i++) {
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
    lastConfigsStatusCheckTime = 0;
    checkResponse(cc.send(new ApplyLocaleResource()).getResp());
  }

  /**
   * Возвращает статус конфигурации ресурсов
   *
   * @return статус конфигурации ресурсов
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public ConfigState getResourcesState() throws AdminException {
    return getConfigState(ConfigType.Resources);
  }

  // ACCESS CONTROL LIST ===============================================================================================

  /**
   * Возвращает информацию обо всех ACL
   *
   * @return список CCAclList, каждый из которых содержит данные об одном ACL
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public List<CCAclInfo> getAcls() throws AdminException {
    AclList req = new AclList();
    AclListResp resp = cc.send(req);

    checkResponse(resp.getResp());

    List<CCAclInfo> result = new ArrayList<CCAclInfo>(resp.getAclList().length);
    for (AclInfo l : resp.getAclList()) {
      CCAclInfo info = new CCAclInfo();
      info.setId(l.getId());
      info.setName(l.getName());
      info.setDescription(l.getDescription());
      result.add(info);
    }
    return result;
  }

  /**
   * Возвращает информацию об ACL по его идентификатору
   *
   * @param aclId идентификатор ACL
   * @return информацию об ACL
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public CCAclInfo getAcl(int aclId) throws AdminException {
    AclGet req = new AclGet();
    req.setAclId(aclId);

    AclGetResp resp = cc.send(req);
    checkResponse(resp.getResp());

    AclInfo acl = resp.getAcl();
    return new CCAclInfo(acl.getId(), acl.getName(), acl.getDescription());
  }

  /**
   * Возвращает список адресов, содержащихся в конкретном ALC по его идентификатору
   *
   * @param aclId идентификатор ACL
   * @return список адресов, содержащихся в ALC
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public List<Address> getAclAddresses(int aclId) throws AdminException {
    AclLookup req = new AclLookup();
    req.setAclId(aclId);
    req.setAddrPrefix("");

    AclLookupResp resp = cc.send(req);

    checkResponse(resp.getResp());

    List<Address> result = new ArrayList<Address>(resp.getResult().length);
    for (String a : resp.getResult())
      result.add(new Address(a));
    return result;
  }

  /**
   * Отправляет команду на создание нового ACL
   *
   * @param aclId       идентификатор
   * @param name        имя
   * @param description описание
   * @param addresses   список адресов
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void createAlc(int aclId, String name, String description, List<Address> addresses) throws AdminException {
    String[] addr = new String[addresses.size()];
    for (int i = 0; i < addresses.size(); i++)
      addr[i] = addresses.get(i).getSimpleAddress();

    AclCreate req = new AclCreate();
    req.setId(aclId);
    req.setName(name);
    req.setDescription(description);
    req.setAddresses(addr);

    checkResponse(cc.send(req).getResp());
  }

  /**
   * Обновляет информацию об ACL
   *
   * @param acl новые данные об ACL
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void updateAcl(CCAclInfo acl) throws AdminException {
    AclUpdate req = new AclUpdate();
    req.setAcl(new AclInfo(acl.getId(), acl.getName(), acl.getDescription()));
    checkResponse(cc.send(req).getResp());
  }

  /**
   * Удаляет ACL
   *
   * @param aclId идентификатор ACL
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void removeAcl(int aclId) throws AdminException {
    AclRemove req = new AclRemove();
    req.setAclId(aclId);
    checkResponse(cc.send(req).getResp());
  }

  /**
   * Добавляет адреса в ACL
   *
   * @param aclId     идентификатор ACL
   * @param addresses список адресов
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void addAddressesToAcl(int aclId, List<Address> addresses) throws AdminException {
    String[] addr = new String[addresses.size()];
    for (int i = 0; i < addresses.size(); i++)
      addr[i] = addresses.get(i).getSimpleAddress();

    AclAddAddresses req = new AclAddAddresses();
    req.setAclId(aclId);
    req.setAddrs(addr);

    checkResponse(cc.send(req).getResp());
  }

  /**
   * Удаляет адреса из ACL
   *
   * @param aclId     идентификатор ACL
   * @param addresses список адресов, которые надо удалить
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void removeAddressesFromAcl(int aclId, List<Address> addresses) throws AdminException {
    String[] addr = new String[addresses.size()];
    for (int i = 0; i < addresses.size(); i++)
      addr[i] = addresses.get(i).getSimpleAddress();

    AclRemoveAddresses req = new AclRemoveAddresses();
    req.setAclId(aclId);
    req.setAddrs(addr);

    checkResponse(cc.send(req).getResp());
  }

  /**
   * Возвращает статус конфигурации ACL
   *
   * @return статус конфигурации ACL
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public ConfigState getAclState() throws AdminException {
    return getConfigState(ConfigType.Acl);
  }

  // ROUTES ==================================================================================================

  /**
   * Блокирует конфигурацию маршрутов для чтения/записи
   *
   * @param write блокировать бля записи
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void lockRoutes(boolean write) throws AdminException {
    lockConfig(ConfigType.Routes, write);
  }

  /**
   * Разблокирует конфигураию маршрутов
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unlockRoutes() throws AdminException {
    unlockConfig(ConfigType.Routes);
  }

  /**
   * Применяет конфигурацию маршрутов
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void applyRoutes() throws AdminException {
    checkResponse(cc.send(new ApplyRoutes()).getResp());
  }

  /**
   * Возвращает статус конфигурации маршрутов
   *
   * @return статус конфигурации маршрутов
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public ConfigState getRoutesState() throws AdminException {
    return getConfigState(ConfigType.Routes);
  }

  /**
   * Отправляет запрос на трассировку маршрута
   *
   * @param fileName    название файла (только имя, файл должен находиться в той же директории, что и routes.xml)
   *                    с тестовой конфигурацией маршрутов
   * @param source      адрес отправителя
   * @param destination адрес получателя
   * @param sourceSmeId Sme Id отправителя
   * @return Экземпляр CCTraceRouteResult
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public CCRouteTrace traceRoute(String fileName, Address source, Address destination, String sourceSmeId) throws AdminException {
    TraceRoute req = new TraceRoute();
    req.setFileName(fileName);
    req.setSrc(source.getSimpleAddress());
    req.setDst(destination.getSimpleAddress());
    req.setSrcSysId(sourceSmeId);

    TraceRouteResp resp = cc.send(req);

    checkResponse(resp.getResp());

    return new CCRouteTrace(resp.getFound() ? resp.getRouteId() : null, resp.getTrace());
  }

  // TIMEZONES ===============================================================================================

  /**
   * Блокирует конфигурацию таймзон для чтения/записи
   *
   * @param write блокировать бля записи
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void lockTimezones(boolean write) throws AdminException {
    lockConfig(ConfigType.TimeZones, write);
  }

  /**
   * Разблокирует конфигурацию таймзон
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unlockTimezones() throws AdminException {
    unlockConfig(ConfigType.TimeZones);
  }

  /**
   * Применяет конфигурацию таймзон
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void applyTimezones() throws AdminException {
    checkResponse(cc.send(new ApplyTimeZones()).getResp());
  }

  /**
   * Возвращает статус конфигурации таймзон
   *
   * @return статус конфигурации таймзон
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public ConfigState getTimezonesState() throws AdminException {
    return getConfigState(ConfigType.TimeZones);
  }

  // PROFILES ================================================================================================

  /**
   * Блокирует конфигурацию профилей для чтения/записи
   *
   * @param write блокировать бля записи
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void lockProfiles(boolean write) throws AdminException {
    lockConfig(ConfigType.Profiles, write);
  }

  /**
   * Разблокирует конфигурацию профилей
   *
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void unlockProfiles() throws AdminException {
    unlockConfig(ConfigType.Profiles);
  }

  /**
   * Ищет профиль по адресу абонента
   *
   * @param address адрес абонента
   * @return экземпляр CCLookupProfileResult
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public CCLookupProfileResult lookupProfile(Address address) throws AdminException {
    LookupProfile req = new LookupProfile();
    req.setAddress(address.getSimpleAddress());
    LookupProfileResp resp = cc.send(req);
    checkResponse(resp.getResp());

    return new CCLookupProfileResult(resp.getProf(), resp.getMatchType());
  }

  /**
   * Обновляет данные профиля
   *
   * @param address адрес или маска
   * @param profile новые данные профиля
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void updateProfile(Address address, CCProfile profile) throws AdminException {
    UpdateProfile req = new UpdateProfile();
    req.setAddress(address.getSimpleAddress());
    req.setProf(profile.toProfile());
    checkResponse(cc.send(req).getResp());
  }

  /**
   * Удаляет профиль по адресу или маске
   *
   * @param address адрес или маска
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void deleteProfile(Address address) throws AdminException {
    DeleteProfile req = new DeleteProfile();
    req.setAddress(address.getSimpleAddress());
    checkResponse(cc.send(req).getResp());
  }

  /**
   * Возвращает статус конфигурации профилей
   *
   * @return статус конфигурации профилей
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public ConfigState getProfilesState() throws AdminException {
    return getConfigState(ConfigType.Profiles);
  }

  // LOGGING =================================================================================================

  /**
   * Возвращает настройки логирования в виде списка логгеров
   *
   * @return список доступных логгеров
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public Collection<CCLoggingInfo> getLoggers() throws AdminException {
    LoggerGetCategoriesResp resp = cc.send(new LoggerGetCategories());

    checkResponse(resp.getResp());

    Collection<CCLoggingInfo> result = new ArrayList<CCLoggingInfo>(resp.getCategories().length);
    for (CategoryInfo i : resp.getCategories())
      result.add(new CCLoggingInfo(i));
    return result;
  }

  /**
   * Обновляет настройки логирования
   *
   * @param loggers новые настройки логирования
   * @throws AdminException если произошла ошибка при взаимодействии с СС
   */
  public void setLoggers(Collection<CCLoggingInfo> loggers) throws AdminException {
    CategoryInfo[] infos = new CategoryInfo[loggers.size()];
    int i = 0;
    for (CCLoggingInfo logger : loggers)
      infos[i++] = logger.toCategoryInfo();

    LoggerSetCategories req = new LoggerSetCategories();
    req.setCategories(infos);

    checkResponse(cc.send(req).getResp());
  }

}

package ru.novosoft.smsc.admin.sme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.CCSme;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.cluster_controller.CCSmeSmscStatuses;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.config.SmscConfiguration;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.ServiceManager;

import java.io.File;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Менеджер, управляющий настройками и статусами SME.
 * Менеджер предоставляет доступ к трем вещам:
 * 1. Список SME
 * 2. Статусы SME с точки зрения SMSC (приконнекчена ли SME к SMSC, в каком режиме и т.п.)
 * 3. Статусы SME с точки зрения демона (на какой ноде запущена и т.п.)
 *
 * @author Artem Snopkov
 */
public class SmeManager implements SmscConfiguration {

  private final ConfigFileManager<Map<String, Sme>> smeFileManager;
  private final ClusterController cc;
  private final ServiceManager serviceManager;
  private boolean broken;
  private Map<String, Sme> smes;

  public SmeManager(File configFile, File backupDir, ClusterController cc, ServiceManager sm, FileSystem fs) throws AdminException {
    this.cc = cc;
    this.serviceManager = sm;
    smeFileManager = new ConfigFileManager<Map<String, Sme>>(configFile, backupDir, fs, new SmeConfigFile());

    try {
      if (cc.isOnline())
        cc.lockSmeConfig(false);
      smes = smeFileManager.load();
    } finally {
      if (cc.isOnline())
        cc.unlockSmeConfig();
    }
  }

  // SMEs list actions =================================================================================================

  private void checkBroken() throws AdminException {
    if (broken) {
      Map<Integer, SmscConfigurationStatus> st = getStatusForSmscs();
      if (st.containsValue(SmscConfigurationStatus.OUT_OF_DATE))
        throw new SmeException("sme_configuration_broken");
      else
        broken = false;
    }
  }

  private void save() throws AdminException {
    try {
      if (cc.isOnline())
        cc.lockSmeConfig(true);
      smeFileManager.save(smes);
    } finally {
      if (cc.isOnline())
        cc.unlockSmeConfig();
    }
  }

  static CCSme sme2CCSme(String smeId, Sme sme) {
    CCSme ccSme = new CCSme();
    ccSme.setId(smeId);
    ccSme.setSystemType(sme.getSystemType());
    ccSme.setAccessMask(sme.getAccessMask());
    ccSme.setAddrRange(sme.getAddrRange());
    ccSme.setDisabled(sme.isDisabled());
    ccSme.setPassword(sme.getPassword());
    ccSme.setPriority(sme.getPriority());
    ccSme.setProcLimit(sme.getProclimit());
    ccSme.setReceiptScheme(sme.getReceiptSchemeName());
    ccSme.setSchedLimit(sme.getSchedlimit());
    ccSme.setSmeN(sme.getSmeN());
    ccSme.setTimeout(sme.getTimeout());
    ccSme.setWantAlias(sme.isWantAlias());
    ccSme.setFlagCarryOrgAbonentInfo(sme.isCarryOrgUserInfo());
    ccSme.setFlagCarryOrgDescriptor(sme.isCarryOrgDescriptor());
    ccSme.setFlagCarrySccpInfo(sme.isCarrySccpInfo());
    ccSme.setFlagFillExtraDescriptor(sme.isFillExtraDescriptor());
    ccSme.setFlagForceGsmDatacoding(sme.isForceGsmDataCoding());
    ccSme.setFlagForceReceiptToSme(sme.isForceSmeReceipt());
    ccSme.setFlagSmppPlus(sme.isSmppPlus());

    switch (sme.getBindMode()) {
      case TX:
        ccSme.setMode(CCSme.BIND_MODE_TX);
        break;
      case RX:
        ccSme.setMode(CCSme.BIND_MODE_RX);
        break;
      case TRX:
        ccSme.setMode(CCSme.BIND_MODE_TRX);
        break;
    }

    return ccSme;
  }


  /**
   * Добавляет новую SME в список или обновляет данные об SME
   *
   * @param smeId  идентификатор SME
   * @param newSme настройки SME
   * @throws AdminException еслипроизошла ошибка
   */
  public void addSme(String smeId, Sme newSme) throws AdminException {
    checkBroken();

    Sme sme = new Sme(newSme);

    boolean exist = smes.containsKey(smeId);
    smes.put(smeId, sme);

    try {
      save();

      CCSme params = sme2CCSme(smeId, sme);

      if (cc.isOnline()) {
        if (exist)
          cc.updateSme(params);
        else
          cc.addSme(params);
      }

    } catch (AdminException e) {
      broken = true;
      throw e;
    }
  }

  /**
   * Возвращает настройки SME по её идентификатору
   *
   * @param smeId идентификатор SME
   * @return Настройки SME или null, если SME с заданным идентификатором не существует.
   * @throws AdminException если произошла ошибка
   */
  public Sme getSme(String smeId) throws AdminException {
    Sme sme = smes.get(smeId);
    if (sme == null)
      return null;
    return new Sme(sme);
  }

  /**
   * Удаляет SME из списка по её идентификатору.
   *
   * @param smeId идентификатор SME.
   * @return true, если SME с таким идентификатором существует.
   * @throws AdminException если произошла ошибка.
   */
  public boolean removeSme(String smeId) throws AdminException {
    checkBroken();

    if (smes.remove(smeId) == null)
      return false;

    try {
      save();

      cc.removeSme(smeId);

    } catch (AdminException e) {
      broken = true;
      throw e;
    }
    return true;
  }

  /**
   * Проверяет, существует ли SME с указанным идентификатором
   *
   * @param smeId идентификатор SME
   * @return true, если SME с указанным идентификатором присутствует в списке.
   */
  public boolean contains(String smeId) {
    return smes.containsKey(smeId);
  }

  /**
   * Возвращает информацию обо всех SME
   *
   * @return список настроек всех SME
   * @throws ru.novosoft.smsc.admin.AdminException
   *          если произошла ошибка
   */
  public Map<String, Sme> smes() throws AdminException {
    Map<String, Sme> result = new HashMap<String, Sme>();
    for (Map.Entry<String, Sme> e : smes.entrySet())
      result.put(e.getKey(), new Sme(e.getValue()));
    return result;
  }

  /**
   * Возвращает состояние конфигурации для всех СМСЦ
   *
   * @return состояние конфигурации для всех СМСЦ
   * @throws AdminException если произошла ошибка
   */
  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    if (!cc.isOnline())
      return null;

    ConfigState configState = cc.getSmeConfigState();

    Map<Integer, SmscConfigurationStatus> result = new HashMap<Integer, SmscConfigurationStatus>();
    if (configState != null) {
      long ccLastUpdateTime = configState.getCcLastUpdateTime();
      for (Map.Entry<Integer, Long> e : configState.getInstancesUpdateTimes().entrySet()) {
        SmscConfigurationStatus s = (e.getValue() >= ccLastUpdateTime) ? SmscConfigurationStatus.UP_TO_DATE : SmscConfigurationStatus.OUT_OF_DATE;
        result.put(e.getKey(), s);
      }
    }
    return result;
  }

  private static SmeSmscStatus connectStatus2smscStatus(CCSmeSmscStatuses.ConnectStatus cs) {
    SmeConnectStatus connSt;
    switch (cs.getStatus()) {
      case CCSme.SME_STATUS_BOUND:
        connSt = SmeConnectStatus.CONNECTED;
        break;
      case CCSme.SME_STATUS_DISCONNECTED:
        connSt = SmeConnectStatus.DISCONNECTED;
        break;
      case CCSme.SME_STATUS_INTERNAL:
        connSt = SmeConnectStatus.INTERNAL;
        break;
      default:
        connSt = null;
    }

    SmeBindMode bm = null;
    if (cs.getBindMode() != null) {
      switch (cs.getBindMode()) {
        case CCSme.BIND_MODE_TX:
          bm = SmeBindMode.TX;
          break;
        case CCSme.BIND_MODE_RX:
          bm = SmeBindMode.RX;
          break;
        case CCSme.BIND_MODE_TRX:
          bm = SmeBindMode.TRX;
          break;
        default:
          bm = null;
      }
    }

    return new SmeSmscStatus(cs.getNodeIndex(), connSt, bm, cs.getPeerIn(), cs.getPeerOut());
  }

  private static SmeSmscStatuses ccSmeSmscStatuses2smeSmscStatuses(CCSmeSmscStatuses info) {
    SmeConnectType ct;
    switch (info.getConnectType()) {
      case CCSme.CONNECT_TYPE_DIRECT:
        ct = SmeConnectType.DIRECT_CONNECT;
        break;
      case CCSme.CONNECT_TYPE_LOAD_BALANCER:
        ct = SmeConnectType.VIA_LOAD_BALANCER;
        break;
      default:
        ct = null;
    }

    SmeSmscStatus s[] = new SmeSmscStatus[info.getConnectStatuses().size()];
    for (int i = 0; i < s.length; i++)
      s[i] = connectStatus2smscStatus(info.getConnectStatuses().get(i));

    return new SmeSmscStatuses(ct, s);
  }

  // SME SMSC status actions ===========================================================================================

  /**
   * Возвращает статусы всех зарегистрированных SME с точки зрения СМС центров
   *
   * @return статус SME в СМСЦ или null, если SME с таким идентификатором не существует
   * @throws AdminException если произошла ошибка
   */
  public Map<String, SmeSmscStatuses> getSmesSmscStatuses() throws AdminException {
    checkBroken();

    CCSmeSmscStatuses stateInfos[] = cc.getSmesStatus();
    Map<String, SmeSmscStatuses> res = new HashMap<String, SmeSmscStatuses>();

    for (CCSmeSmscStatuses stateInfo : stateInfos)
      res.put(stateInfo.getSmeId(), ccSmeSmscStatuses2smeSmscStatuses(stateInfo));

    return res;
  }

  /**
   * Отключает SME с указанными идентификаторами от всех инстанцев SMSC при помощи SMPP UNBIND
   *
   * @param smeIds идентификаторы SME-х
   * @throws AdminException если произошла ошибка
   */
  public void disconnectSmeFromSmsc(String... smeIds) throws AdminException {
    checkBroken();
    cc.disconnectSmes(smeIds);
  }

  /**
   * Отключает SME с указанными идентификаторами от всех инстанцев SMSC при помощи SMPP UNBIND
   *
   * @param smeIds идентификаторы SME-х
   * @throws AdminException если произошла ошибка
   */
  public void disconnectSmeFromSmsc(List<String> smeIds) throws AdminException {
    checkBroken();
    String[] strings = new String[smeIds.size()];
    for (int i = 0; i < smeIds.size(); i++)
      strings[i] = smeIds.get(i);
    cc.disconnectSmes(strings);
  }

  // SME daemon status actions =========================================================================================

  /**
   * Возвращает статус SME с точки зрения демона.
   *
   * @param smeId идентификатор SME
   * @return статус SME в демоне или null, если SME не зарегистрирована в демоне
   * @throws AdminException если произошла ошибка
   */
  public SmeServiceStatus getSmeServiceStatus(String smeId) throws AdminException {
    ServiceInfo serviceInfo = serviceManager.getService(smeId);
    if (serviceInfo == null)
      return null;

    return new SmeServiceStatus(serviceInfo.getOnlineHost(), serviceInfo.getHosts());
  }

  /**
   * Запускает SME на дефолтном хосте
   *
   * @param smeId идентификатор SME
   * @throws AdminException если произошла ошибка
   */
  public void startSme(String smeId) throws AdminException {
    serviceManager.startService(smeId);
  }

  /**
   * Останавливает SME
   *
   * @param smeId идентификатор SME
   * @throws AdminException если произошла ошибка
   */
  public void stopSme(String smeId) throws AdminException {
    serviceManager.stopService(smeId);
  }

  /**
   * Переключает SME на указанный хост. Список хостов, на которых может быть запущена SME,
   * можно узнать из SmeServiceStatus.
   *
   * @param smeId  идентификатор SME
   * @param toHost хост, на который надо переключить SME
   * @throws AdminException если произошла ошибка
   */
  public void switchSme(String smeId, String toHost) throws AdminException {
    serviceManager.swichService(smeId, toHost);
  }

}

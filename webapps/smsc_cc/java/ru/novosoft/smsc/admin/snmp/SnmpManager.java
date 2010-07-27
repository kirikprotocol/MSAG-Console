package ru.novosoft.smsc.admin.snmp;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.config.SmscConfiguration;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.util.ValidationHelper;

import java.io.File;
import java.util.HashMap;
import java.util.Map;

/**
 * Менеджер, управляющий конфигурацией Snmp
 * @author Artem Snopkov
 */
public class SnmpManager extends ConfigFileManager<SnmpConfigFile> implements SmscConfiguration {

  private static final ValidationHelper vh = new ValidationHelper(SnmpManager.class);

  private final ClusterController cc;

  public SnmpManager(File configFile, File backupDir, ClusterController cc, FileSystem fileSystem) {
    super(configFile, backupDir, fileSystem);
    this.cc = cc;
  }

  /**
   * Возвращает Counter Interval
   *
   * @return Counter Interval
   */
  public int getCounterInterval() {
    return config.getCounterInterval();
  }

  /**
   * Задает Counter Interval
   *
   * @param counterInterval Counter Interval
   */
  public void setCounterInterval(int counterInterval) throws AdminException {
    vh.checkPositive("counterInterval", counterInterval);
    config.setCounterInterval(counterInterval);
    setChanged();
  }

  /**
   * Возвращает Snmp объект по-умолчанию
   *
   * @return Snmp объект по-умолчанию
   */
  public SnmpObject getDefaultSnmpObject() {
    return new SnmpObject(config.getDefaultSnmpObject());
  }

  /**
   * Задает Snmp объект по-умолчанию
   *
   * @param defaultSnmpObject Snmp объект по-умолчанию
   */
  public void setDefaultSnmpObject(SnmpObject defaultSnmpObject) throws AdminException {
    config.setDefaultSnmpObject(new SnmpObject(defaultSnmpObject));
    setChanged();
  }

  /**
   * Возвращает Map, в котором ключем выступает идентификатор SnmpObject, значением - сам объект.
   *
   * @return Map, в котором ключем выступает идентификатор SnmpObject, значением - сам объект.
   */
  public Map<String, SnmpObject> getSnmpObjects() {
    HashMap<String, SnmpObject> res = new HashMap<String, SnmpObject>();
    for (Map.Entry<String, SnmpObject> e : config.getSnmpObjects().entrySet())
      res.put(e.getKey(), new SnmpObject(e.getValue()));
    return res;
  }

  /**
   * Задает новый список SNMP объектов.
   *
   * @param snmpObjects Map, в котором ключем является идентификатор объекта, значением - сам объект
   */
  public void setSnmpObjects(Map<String, SnmpObject> snmpObjects) throws AdminException {
    vh.checkNoNulls("snmpObjects", snmpObjects);

    HashMap<String, SnmpObject> res = new HashMap<String, SnmpObject>();
    for (Map.Entry<String, SnmpObject> e : snmpObjects.entrySet())
      res.put(e.getKey(), new SnmpObject(e.getValue()));
    config.setSnmpObjects(res);
    setChanged();
  }

  @Override
  protected SnmpConfigFile newConfigFile() {
    return new SnmpConfigFile();
  }

  @Override
  protected void lockConfig(boolean write) throws AdminException {
    cc.lockSnmp(write);
  }

  @Override
  protected void unlockConfig() throws Exception {
    cc.unlockSnmp();
  }

  @Override
  protected void afterApply() throws AdminException {
    cc.applySnmp();
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    ConfigState state = cc.getSnmpConfigState();
    long lastUpdate = configFile.lastModified();
    Map<Integer, SmscConfigurationStatus> result = new HashMap<Integer, SmscConfigurationStatus>();
    for (Map.Entry<Integer, Long> e : state.getInstancesUpdateTimes().entrySet()) {
      SmscConfigurationStatus s = e.getValue() >= lastUpdate ? SmscConfigurationStatus.UP_TO_DATE : SmscConfigurationStatus.OUT_OF_DATE;
      result.put(e.getKey(), s);
    }
    return result;
  }
}

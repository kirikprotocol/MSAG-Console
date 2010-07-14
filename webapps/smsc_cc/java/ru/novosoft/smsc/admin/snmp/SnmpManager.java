package ru.novosoft.smsc.admin.snmp;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;
import java.util.Map;

/**
 * Менеджер, управляющий конфигурацией Snmp
 * @author Artem Snopkov
 */
public class SnmpManager extends ConfigFileManager<SnmpConfigFile> {

  private final ClusterController cc;

  public SnmpManager(File configFile, File backupDir, ClusterController cc, FileSystem fileSystem) throws AdminException {
    super(configFile, backupDir, fileSystem);
    this.cc = cc;
    reset();
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
  public void setCounterInterval(int counterInterval) {
    config.setCounterInterval(counterInterval);
    changed = true;
  }

  /**
   * Возвращает Snmp объект по-умолчанию
   *
   * @return Snmp объект по-умолчанию
   */
  public SnmpObject getDefaultSnmpObject() {
    return config.getDefaultSnmpObject();
  }

  /**
   * Задает Snmp объект по-умолчанию
   *
   * @param defaultSnmpObject Snmp объект по-умолчанию
   */
  public void setDefaultSnmpObject(SnmpObject defaultSnmpObject) {
    config.setDefaultSnmpObject(defaultSnmpObject);
    changed = true;
  }

  /**
   * Возвращает Map, в котором ключем выступает идентификатор SnmpObject, значением - сам объект.
   *
   * @return Map, в котором ключем выступает идентификатор SnmpObject, значением - сам объект.
   */
  public Map<String, SnmpObject> getSnmpObjects() {
    return config.getSnmpObjects();
  }

  /**
   * Задает новый список SNMP объектов.
   *
   * @param snmpObjects Map, в котором ключем является идентификатор объекта, значением - сам объект
   */
  public void setSnmpObjects(Map<String, SnmpObject> snmpObjects) {
    config.setSnmpObjects(snmpObjects);
    changed = true;
  }

  @Override
  protected SnmpConfigFile newConfigFile() {
    return new SnmpConfigFile();
  }

  @Override
  protected void afterApply() throws AdminException {
    cc.applySnmp();
  }
}

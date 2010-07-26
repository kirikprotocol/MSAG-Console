package ru.novosoft.smsc.admin.smsc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.config.SmscConfiguration;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
@SuppressWarnings({"EmptyCatchBlock"})
public class SmscManager extends ConfigFileManager<SmscConfigFile> implements SmscConfiguration {


  private List<SmscConfigObserver> observers = new ArrayList<SmscConfigObserver>();
  private final ClusterController cc;

  public SmscManager(File configFile, File backupDir, ClusterController cc, FileSystem fileSystem) throws AdminException {
    super(configFile, backupDir, fileSystem);
    this.cc = cc;
    reset();
  }

  @Override
  protected SmscConfigFile newConfigFile() {
    return new SmscConfigFile();
  }

  /**
   * Возвращает настройки (копию), общие для всех СМСЦ.
   *
   * @return настройки, общие для всех СМСЦ.
   */
  public CommonSettings getCommonSettings() {
    try {
      return (CommonSettings) config.getCommonSettings().clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
  }

  /**
   * Устанавливает новые значения для общих настроек
   *
   * @param settings новые значения общих настроек
   * @throws AdminException если новые настройки содержат ошибки
   */
  public void setCommonSettings(CommonSettings settings) throws AdminException {
    for (SmscConfigObserver l : observers) {
      l.setCommonSettings(settings);
    }
    try {
      config.setCommonSettings((CommonSettings) settings.clone());
    } catch (CloneNotSupportedException e) {
    }
    changed = true;
  }


  /**
   * Устанавливает новые значения для настроек инстанца СМСЦ
   *
   * @param instanceNumber   номер инстанца СМСЦ
   * @param instanceSettings новые настройки
   * @throws AdminException если новые настройки содержат ошибки
   */
  public void setInstanceSettings(int instanceNumber, InstanceSettings instanceSettings) throws AdminException {
    for (SmscConfigObserver l : observers) {
      l.setInstanceSettings(instanceSettings);
    }
    try {
      config.setInstanceSettings(instanceNumber, (InstanceSettings) instanceSettings.clone());
    } catch (CloneNotSupportedException e) {
    }
    changed = true;
  }

  /**
   * Возвращает количество инстанцев СМСЦ
   *
   * @return количество инстанцев СМСЦ
   */
  public int getSmscInstancesCount() {
    return config.getSmscInstancesCount();
  }


  /**
   * Возвращает настройки, специфические для указанного инстанца СМСЦ
   *
   * @param instanceNumber номер инстанца
   * @return настройки, специфические для каждого инстанца СМСЦ
   */
  public InstanceSettings getInstanceSettings(int instanceNumber) {
    try {
      return (InstanceSettings) config.getInstanceSettings(instanceNumber).clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
  }


  @Override
  protected void beforeApply() throws AdminException {
    for (SmscConfigObserver o : observers)
      o.applySettings(config.getCommonSettings(), config.getAllInstanceSettings());
  }

  @Override
  protected void beforeReset() throws AdminException {
    for (SmscConfigObserver o : observers)
      o.resetSettings(config.getCommonSettings(), config.getAllInstanceSettings());
  }

  @Override
  protected void lockConfig(boolean write) throws AdminException {
    cc.lockMainConfig(write);
  }

  @Override
  protected void unlockConfig() throws Exception {
    cc.unlockMainConfig();
  }

  /**
   * Добавляет слушателя конфига
   *
   * @param l слушатель
   */
  public void addObserver(SmscConfigObserver l) {
    observers.add(l);
  }

  /**
   * Удаляет слушателя конфига
   *
   * @param l слушатель
   */
  public void removeObserver(SmscConfigObserver l) {
    observers.remove(l);
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    ConfigState state = cc.getMainConfigState();
    long lastUpdate = configFile.lastModified();
    Map<Integer, SmscConfigurationStatus> result = new HashMap<Integer, SmscConfigurationStatus>();
    for (Map.Entry<Integer, Long> e : state.getInstancesUpdateTimes().entrySet()) {
      SmscConfigurationStatus s = e.getValue() >= lastUpdate ? SmscConfigurationStatus.UP_TO_DATE : SmscConfigurationStatus.OUT_OF_DATE;
      result.put(e.getKey(), s);
    }
    return result;
  }
}

package ru.novosoft.smsc.admin.smsc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author Artem Snopkov
 */
@SuppressWarnings({"EmptyCatchBlock"})
public class SmscConfig {

  private SmscConfigFile configFile;

  private final FileSystem fileSystem;

  private boolean changed;

  private List<SmscConfigObserver> observers = new ArrayList<SmscConfigObserver>();

  public SmscConfig(File configFile, File backupDir, FileSystem fileSystem) throws AdminException {
    this.configFile = createConfigFile(configFile, backupDir, fileSystem);
    this.fileSystem = fileSystem;
    reset();
  }

  /**
   * Возвращает настройки (копию), общие для всех СМСЦ.
   *
   * @return настройки, общие для всех СМСЦ.
   */
  public CommonSettings getCommonSettings()  {
    try {
      return (CommonSettings) configFile.getCommonSettings().clone();
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
      configFile.setCommonSettings((CommonSettings)settings.clone());
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
      configFile.setInstanceSettings(instanceNumber, (InstanceSettings)instanceSettings.clone());
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
    return configFile.getSmscInstancesCount();
  }


  /**
   * Возвращает настройки, специфические для указанного инстанца СМСЦ
   *
   * @param instanceNumber номер инстанца
   * @return настройки, специфические для каждого инстанца СМСЦ
   */
  public InstanceSettings getInstanceSettings(int instanceNumber) {
    try {
      return (InstanceSettings) configFile.getInstanceSettings(instanceNumber).clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
  }

  /**
   * Сохраняет и применяет изменения, сделанные в конфиге
   *
   * @throws AdminException ошибка при сохранении конфига
   */
  public synchronized void save() throws AdminException {
    for (SmscConfigObserver l : observers) {
      l.applySettings(configFile.getCommonSettings(), configFile.getAllInstanceSettings());
    }
    configFile.save();
    changed = false;
  }

  protected SmscConfigFile createConfigFile(File smscConfigFile, File backupDir, FileSystem fileSystem) {
    return new SmscConfigFile(smscConfigFile, backupDir, fileSystem);
  }

  /**
   * Откатывает изменения, сделанные в конфиге
   *
   * @throws AdminException если откатить конфиг невозможно.
   */
  public synchronized void reset() throws AdminException {

    SmscConfigFile oldConfigFile = createConfigFile(configFile.getSmscConfigFile(), configFile.getBackupDir(), fileSystem);
    oldConfigFile.load();
    for (SmscConfigObserver l : observers) {
      l.resetSettings(oldConfigFile.getCommonSettings(), oldConfigFile.getAllInstanceSettings());
    }
    this.configFile = oldConfigFile;
    changed = false;
  }

  /**
   * Метод возвращает true, если конфиг менялся, false - иначе
   *
   * @return true - конфиг менялся, false - иначе
   */
  public boolean isChanged() {
    return changed;
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
}

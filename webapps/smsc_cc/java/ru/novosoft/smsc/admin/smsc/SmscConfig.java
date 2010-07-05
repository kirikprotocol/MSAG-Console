package ru.novosoft.smsc.admin.smsc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author Artem Snopkov
 */
@SuppressWarnings({"EmptyCatchBlock"})
public class SmscConfig {

  private SmscConfigFile configFile;

  private final File smscBaseDir;
  private final FileSystem fileSystem;

  private boolean changed;

  private List<SmscConfigObserver> observers = new ArrayList<SmscConfigObserver>();

  /**
   * @param smscBaseDir директория с дистрибутивом СМСЦ
   * @param fileSystem экземпляр FileSystem
   * @throws AdminException ошибка при чтении конфига
   */
  public SmscConfig(File smscBaseDir, FileSystem fileSystem) throws AdminException {
    this.configFile = new SmscConfigFile(smscBaseDir, fileSystem);
    this.smscBaseDir = smscBaseDir;
    this.fileSystem = fileSystem;
    reset();    
  }

  /**
   * Возвращает настройки (копию), общие для всех СМСЦ.
   *
   * @return настройки, общие для всех СМСЦ.
   * @throws AdminException ошибка извлечения настроек
   */
  public CommonSettings getCommonSettings() throws AdminException {
    try {
      return (CommonSettings) configFile.getCommonSettings().clone();
    } catch (CloneNotSupportedException e) {
      throw new AdminException(e.getMessage());
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
      throw new AdminException(e.getMessage(), e);
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
      configFile.setInstanceSettings((InstanceSettings)instanceSettings.clone());
    } catch (CloneNotSupportedException e) {
      throw new AdminException(e.getMessage(), e);
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
   * @throws AdminException ошибка извлечения настроек
   */
  public InstanceSettings getInstanceSettings(int instanceNumber) throws AdminException {
    try {
      return (InstanceSettings) configFile.getInstanceSettings(instanceNumber).clone();
    } catch (CloneNotSupportedException e) {
      throw new AdminException(e.getMessage());
    }
  }

  private final SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yy.HH.mm.ss");

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

  /**
   * Откатывает изменения, сделанные в конфиге
   *
   * @throws AdminException если откатить конфиг невозможно.
   */
  public synchronized void reset() throws AdminException {

    SmscConfigFile oldConfigFile = new SmscConfigFile(smscBaseDir, fileSystem);
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

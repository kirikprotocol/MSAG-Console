package ru.novosoft.smsc.admin.smsc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author Artem Snopkov
 */
@SuppressWarnings({"EmptyCatchBlock"})
public class SmscManager extends ConfigFileManager<SmscConfigFile> {


  private List<SmscConfigObserver> observers = new ArrayList<SmscConfigObserver>();

  public SmscManager(File configFile, File backupDir, FileSystem fileSystem) throws AdminException {
    super(configFile, backupDir, fileSystem);
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
  public CommonSettings getCommonSettings()  {
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
      config.setCommonSettings((CommonSettings)settings.clone());
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
      config.setInstanceSettings(instanceNumber, (InstanceSettings)instanceSettings.clone());
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

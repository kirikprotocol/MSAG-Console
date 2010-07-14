package ru.novosoft.smsc.admin.map_limit;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class MapLimitManager extends ConfigFileManager<MapLimitConfig> {

  public static final int MAX_CONGESTON_LEVELS = MapLimitConfig.MAX_CONGESTION_LEVELS; 

  private final ClusterController cc;

  public MapLimitManager(File configFile, File backupDir, ClusterController cc, FileSystem fs) throws AdminException {
    super(configFile, backupDir, fs);
    this.cc = cc;
    reset();
  }

  /**
   * Возвращает количество уровней перегрузки.
   *
   * @return количество уровней перегрузки
   */
  public int getContestionLevenCount() {
    return MapLimitConfig.MAX_CONGESTION_LEVELS;
  }

  /**
   * Возвращает настройки для уровней перегрузки. Размер массива
   * равен значению, возвращаемому методом getCongestionLevel().
   *
   * @return настройки для уровней перегрузки.
   */
  public CongestionLevel[] getCongestionLevels() {
    return config.getClevels();
  }

  /**
   * Устанавливает новые настройки для уровней перегрузки.
   *
   * @param levels новые настройки для уровней перегрузки. Размер массива должен быть равен getCongessionLevelsCount().
   * @throws AdminException если настройки некорректны
   */
  public void setCongestionLevels(CongestionLevel[] levels)  {
    config.setClevels(levels);
    changed = true;
  }

  public int getDlgLimitIn() {
    return config.getDlgLimitIn();
  }

  public void setDlgLimitIn(int dlgLimitIn) {
    config.setDlgLimitIn(dlgLimitIn);
    changed = true;
  }

  public int getDlgLimitInSri() {
    return config.getDlgLimitInSri();
  }

  public void setDlgLimitInSri(int dlgLimitInSri) {
    config.setDlgLimitInSri(dlgLimitInSri);
    changed = true;
  }

  public int getDlgLimitInUssd() {
    return config.getDlgLimitInUssd();
  }

  public void setDlgLimitInUssd(int dlgLimitInUssd) {
    config.setDlgLimitInUssd(dlgLimitInUssd);
    changed = true;
  } 

  public int getDlgLimitOutSri() {
    return config.getDlgLimitOutSri();
  }

  public void setDlgLimitOutSri(int dlgLimitOutSri) {
    config.setDlgLimitOutSri(dlgLimitOutSri);
    changed = true;
  }

  public int getDlgLimitUssd() {
    return config.getDlgLimitUssd();
  }

  public void setDlgLimitUssd(int dlgLimitUssd) {
    config.setDlgLimitUssd(dlgLimitUssd);
    changed = true;
  }

  public int[] getUssdNoSriCodes() {
    return config.getUssdNoSriCodes();
  }

  public void setUssdNoSriCodes(int[] ussdNoSriCodes) {
    config.setUssdNoSriCodes(ussdNoSriCodes);
    changed = true;
  }

  @Override
  protected MapLimitConfig newConfigFile() {
    return new MapLimitConfig();
  }

  @Override
  protected void afterApply() throws AdminException {
    cc.applyMapLimits();
  }
}

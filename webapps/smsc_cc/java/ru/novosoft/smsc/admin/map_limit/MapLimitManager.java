package ru.novosoft.smsc.admin.map_limit;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.util.ValidationHelper;

import java.io.File;
import java.util.Arrays;

/**
 * @author Artem Snopkov
 */
public class MapLimitManager extends ConfigFileManager<MapLimitConfig> {

  static final int MAX_CONGESTON_LEVELS = MapLimitConfig.MAX_CONGESTION_LEVELS;
  private static final ValidationHelper vh = new ValidationHelper(MapLimitManager.class.getCanonicalName());

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
    return MAX_CONGESTON_LEVELS;
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
  public void setCongestionLevels(CongestionLevel[] levels) throws AdminException {
    vh.checkNoNulls("congestionLevels", Arrays.asList(levels));
    config.setClevels(levels);
    changed = true;
  }

  public int getDlgLimitIn() {
    return config.getDlgLimitIn();
  }

  public void setDlgLimitIn(int dlgLimitIn) throws AdminException {
    vh.checkPositive("dlgLimitIn", dlgLimitIn);
    config.setDlgLimitIn(dlgLimitIn);
    changed = true;
  }

  public int getDlgLimitInSri() {
    return config.getDlgLimitInSri();
  }

  public void setDlgLimitInSri(int dlgLimitInSri) throws AdminException {
    vh.checkPositive("dlgLimitInSri", dlgLimitInSri);
    config.setDlgLimitInSri(dlgLimitInSri);
    changed = true;
  }

  public int getDlgLimitInUssd() {
    return config.getDlgLimitInUssd();
  }

  public void setDlgLimitInUssd(int dlgLimitInUssd) throws AdminException {
    vh.checkPositive("dlgLimitInUssd", dlgLimitInUssd);
    config.setDlgLimitInUssd(dlgLimitInUssd);
    changed = true;
  } 

  public int getDlgLimitOutSri() {
    return config.getDlgLimitOutSri();
  }

  public void setDlgLimitOutSri(int dlgLimitOutSri) throws AdminException {
    vh.checkPositive("dlgLimitOutSri", dlgLimitOutSri);
    config.setDlgLimitOutSri(dlgLimitOutSri);
    changed = true;
  }

  public int getDlgLimitUssd() {
    return config.getDlgLimitUssd();
  }

  public void setDlgLimitUssd(int dlgLimitUssd) throws AdminException {
    vh.checkPositive("dlgLimitUssd", dlgLimitUssd);
    config.setDlgLimitUssd(dlgLimitUssd);
    changed = true;
  }

  public int[] getUssdNoSriCodes() {
    return config.getUssdNoSriCodes();
  }

  public void setUssdNoSriCodes(int[] ussdNoSriCodes) throws AdminException {
    vh.checkPositive("ussdNoSriCodes", ussdNoSriCodes);
    config.setUssdNoSriCodes(ussdNoSriCodes);
    changed = true;
  }

  @Override
  protected MapLimitConfig newConfigFile() {
    return new MapLimitConfig();
  }

  @Override
  protected void lockConfig(boolean write) throws AdminException {
    cc.lockMapLimits(write);
  }

  @Override
  protected void unlockConfig() throws Exception {
    cc.unlockMapLimits();
  }


  @Override
  protected void afterApply() throws AdminException {
    cc.applyMapLimits();
  }
}

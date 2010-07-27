package ru.novosoft.smsc.admin.fraud;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.config.SmscConfiguration;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.util.ValidationHelper;

import java.io.File;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

/**
 * Класс для работы с конфигурацией Fraud
 *
 * @author Artem Snopkov
 */
public class FraudManager extends ConfigFileManager<FraudConfigFile> implements SmscConfiguration {

  private static final ValidationHelper vh = new ValidationHelper(FraudManager.class);

  private ClusterController cc;

  public FraudManager(File configFile, File backupDir, ClusterController cc, FileSystem fs) {
    super(configFile, backupDir, fs);
    this.cc = cc;
  }

  public int getTail() {
    return config.getTail();
  }

  public void setTail(int tail) throws AdminException {
    vh.checkPositive("tail", tail);
    config.setTail(tail);
    setChanged();
  }

  public boolean isEnableCheck() {
    return config.isEnableCheck();
  }

  public void setEnableCheck(boolean enableCheck) {
    config.setEnableCheck(enableCheck);
    setChanged();
  }

  public boolean isEnableReject() {
    return config.isEnableReject();
  }

  public void setEnableReject(boolean enableReject) {
    config.setEnableReject(enableReject);
    setChanged();
  }

  public Collection<String> getWhiteList() {
    return config.getWhiteList();
  }

  public void setWhiteList(Collection<String> whiteList) throws AdminException {
    vh.checkNoNulls("whiteList", whiteList);
    config.setWhiteList(whiteList);
    setChanged();
  }

  @Override
  protected FraudConfigFile newConfigFile() {
    return new FraudConfigFile();
  }

  @Override
  protected void lockConfig(boolean write) throws AdminException {
    cc.lockFraud(write);
  }

  @Override
  protected void unlockConfig() throws Exception {
    cc.unlockFraud();
  }

  @Override
  protected void afterApply() throws AdminException {
    cc.applyFraud();
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    ConfigState state = cc.getFraudConfigState();
    long lastUpdate = configFile.lastModified();
    Map<Integer, SmscConfigurationStatus> result = new HashMap<Integer, SmscConfigurationStatus>();
    for (Map.Entry<Integer, Long> e : state.getInstancesUpdateTimes().entrySet()) {
      SmscConfigurationStatus s = e.getValue() >= lastUpdate ? SmscConfigurationStatus.UP_TO_DATE : SmscConfigurationStatus.OUT_OF_DATE;
      result.put(e.getKey(), s);
    }
    return result;
  }
}

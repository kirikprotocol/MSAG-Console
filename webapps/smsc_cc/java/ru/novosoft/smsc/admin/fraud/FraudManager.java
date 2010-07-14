package ru.novosoft.smsc.admin.fraud;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;
import java.util.Collection;

/**
 * Класс для работы с конфигурацией Fraud
 * @author Artem Snopkov
 */
public class FraudManager extends ConfigFileManager<FraudConfigFile> {

  private ClusterController cc;

  public FraudManager(File configFile, File backupDir, ClusterController cc, FileSystem fs) throws AdminException {
    super(configFile, backupDir, fs);
    this.cc = cc;
    reset();
  }

  public int getTail() {
    return config.getTail();
  }

  public void setTail(int tail) {
    config.setTail(tail);
    changed=true;
  }

  public boolean isEnableCheck() {
    return config.isEnableCheck();
  }

  public void setEnableCheck(boolean enableCheck) {
    config.setEnableCheck(enableCheck);
    changed=true;
  }

  public boolean isEnableReject() {
    return config.isEnableReject();
  }

  public void setEnableReject(boolean enableReject) {
    config.setEnableReject(enableReject);
    changed=true;
  }

  public Collection<String> getWhiteList() {
    return config.getWhiteList();
  }
  
  public void setWhiteList(Collection<String> whiteList) {
    config.setWhiteList(whiteList);
    changed=true;
  }

  @Override
  protected FraudConfigFile newConfigFile() {
    return new FraudConfigFile();
  }

  @Override
  protected void afterApply() throws AdminException {
    cc.applyFraud();
  }
}

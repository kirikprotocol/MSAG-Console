package ru.novosoft.smsc.admin.fraud;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.config.SmscConfiguration;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;
import java.util.HashMap;
import java.util.Map;

/**
 * Класс для работы с конфигурацией Fraud
 *
 * @author Artem Snopkov
 */
public class FraudManager implements SmscConfiguration {

  private ClusterController cc;
  private ConfigFileManager<FraudSettings> cfgFileManager;

  public FraudManager(File configFile, File backupDir, ClusterController cc, FileSystem fs) throws AdminException {
    this.cc = cc;
    this.cfgFileManager = new ConfigFileManager<FraudSettings>(configFile, backupDir, fs, new FraudConfigFile());
  }

  public FraudSettings getSettings() throws AdminException {
    try {
      if (cc.isOnline())
        cc.lockFraud(false);
      return this.cfgFileManager.load();
    } finally {
      if (cc.isOnline())
        cc.unlockFraud();
    }
  }

  public void updateSettings(FraudSettings newSettings) throws AdminException {
    try {
      if (cc.isOnline())
        cc.lockFraud(true);
      cfgFileManager.save(newSettings);
    } finally {
      if (cc.isOnline())
        cc.unlockFraud();
    }

    if (cc.isOnline())
      cc.applyFraud();
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    ConfigState state = cc.getFraudConfigState();
    long lastUpdate = cfgFileManager.getLastModified();
    Map<Integer, SmscConfigurationStatus> result = new HashMap<Integer, SmscConfigurationStatus>();
    for (Map.Entry<Integer, Long> e : state.getInstancesUpdateTimes().entrySet()) {
      SmscConfigurationStatus s = e.getValue() >= lastUpdate ? SmscConfigurationStatus.UP_TO_DATE : SmscConfigurationStatus.OUT_OF_DATE;
      result.put(e.getKey(), s);
    }
    return result;
  }
}

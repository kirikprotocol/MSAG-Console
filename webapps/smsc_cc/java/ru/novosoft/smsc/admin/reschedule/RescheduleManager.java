package ru.novosoft.smsc.admin.reschedule;

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
 * @author Artem Snopkov
 */
public class RescheduleManager implements SmscConfiguration {

  private final ClusterController cc;
  private final ConfigFileManager<RescheduleSettings> cfgFileManager;

  public RescheduleManager(File configFile, File backupDir, ClusterController cc, FileSystem fs) throws AdminException {
    this.cc = cc;
    this.cfgFileManager = new ConfigFileManager<RescheduleSettings>(configFile, backupDir, fs, new RescheduleConfig());
  }

  public RescheduleSettings getSettings() throws AdminException {
    try {
      if (cc.isOnline())
        cc.lockReschedule(false);
      return this.cfgFileManager.load();
    } finally {
      if (cc.isOnline())
        cc.unlockReschedule();
    }
  }

  public void updateSettings(RescheduleSettings settings) throws AdminException {
    try {
      if (cc.isOnline())
        cc.lockReschedule(true);
      cfgFileManager.save(settings);
    } finally {
      if (cc.isOnline())
        cc.unlockReschedule();
    }

    if (cc.isOnline())
      cc.applyReschedule();
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    ConfigState state = cc.getRescheduleConfigState();
    long lastUpdate = cfgFileManager.getLastModified();
    Map<Integer, SmscConfigurationStatus> result = new HashMap<Integer, SmscConfigurationStatus>();
    for (Map.Entry<Integer, Long> e : state.getInstancesUpdateTimes().entrySet()) {
      SmscConfigurationStatus s = e.getValue() >= lastUpdate ? SmscConfigurationStatus.UP_TO_DATE : SmscConfigurationStatus.OUT_OF_DATE;
      result.put(e.getKey() , s);
    }
    return result;
  }


}

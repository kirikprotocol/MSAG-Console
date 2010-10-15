package ru.novosoft.smsc.admin.timezone;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;

import java.io.File;
import java.util.HashMap;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class TimezoneManagerImpl implements TimezoneManager {

  private final ConfigFileManager<TimezoneSettings> cfgFileManager;
  private final ClusterController cc;

  public TimezoneManagerImpl(File configFile, File backupDir, FileSystem fileSystem, ClusterController cc) {
    this.cfgFileManager = new ConfigFileManager<TimezoneSettings>(configFile, backupDir, fileSystem, new TimezonesConfig());
    this.cc = cc;
  }

  public TimezoneSettings getSettings() throws AdminException {
    try {
      if (cc.isOnline())
        cc.lockTimezones(false);

      return cfgFileManager.load();
    } finally {
      if (cc.isOnline())
        cc.unlockTimezones();
    }
  }

  public void updateSettings(TimezoneSettings settings) throws AdminException {
    try {
      if (cc.isOnline())
        cc.lockTimezones(false);

      cfgFileManager.save(settings);

      if (cc.isOnline())
        cc.applyTimezones();
    } finally {
      if (cc.isOnline())
        cc.unlockTimezones();
    }
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    if (!cc.isOnline())
      return null;
    ConfigState state = cc.getTimezonesState();
    Map<Integer, SmscConfigurationStatus> result = new HashMap<Integer, SmscConfigurationStatus>();
    if (state != null) {
      long lastUpdate = cfgFileManager.getLastModified();
      for (Map.Entry<Integer, Long> e : state.getInstancesUpdateTimes().entrySet()) {
        SmscConfigurationStatus s = e.getValue() >= lastUpdate ? SmscConfigurationStatus.UP_TO_DATE : SmscConfigurationStatus.OUT_OF_DATE;
        result.put(e.getKey(), s);
      }
    }
    return result;
  }
}

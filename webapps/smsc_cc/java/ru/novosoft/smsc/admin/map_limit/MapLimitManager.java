package ru.novosoft.smsc.admin.map_limit;

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
public class MapLimitManager implements SmscConfiguration {

  static final int MAX_CONGESTON_LEVELS = MapLimitConfig.MAX_CONGESTION_LEVELS;

  private final ClusterController cc;
  private final ConfigFileManager<MapLimitSettings> cfgFileManager;
  private MapLimitSettings currentSettings;

  public MapLimitManager(File configFile, File backupDir, ClusterController cc, FileSystem fs) throws AdminException {
    this.cc = cc;
    this.cfgFileManager = new ConfigFileManager<MapLimitSettings>(configFile, backupDir, fs, new MapLimitConfig());

    try {
      cc.lockMapLimits(false);
      currentSettings = cfgFileManager.load();
    } finally {
      cc.unlockMapLimits();
    }
  }

  public MapLimitSettings getSettings() {
    return new MapLimitSettings(currentSettings);
  }

  public void updateSettings(MapLimitSettings settings) throws AdminException {
    try {
      cc.lockMapLimits(true);
      cfgFileManager.save(settings);
    } finally {
      cc.unlockMapLimits();
    }

    currentSettings = new MapLimitSettings(settings);

    cc.applyMapLimits();
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    ConfigState state = cc.getMapLimitConfigState();
    long lastUpdate = cfgFileManager.getLastModified();
    Map<Integer, SmscConfigurationStatus> result = new HashMap<Integer, SmscConfigurationStatus>();
    for (Map.Entry<Integer, Long> e : state.getInstancesUpdateTimes().entrySet()) {
      SmscConfigurationStatus s = e.getValue() >= lastUpdate ? SmscConfigurationStatus.UP_TO_DATE : SmscConfigurationStatus.OUT_OF_DATE;
      result.put(e.getKey(), s);
    }
    return result;
  }
}

package ru.novosoft.smsc.admin.map_limit;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.config.SmscConfiguration;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.util.ValidationHelper;

import java.io.File;
import java.util.HashMap;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class MapLimitManager implements SmscConfiguration {

  static final int MAX_CONGESTON_LEVELS = MapLimitConfig.MAX_CONGESTION_LEVELS;
  private static final ValidationHelper vh = new ValidationHelper(MapLimitManager.class);

  private final ClusterController cc;
  private final ConfigFileManager<MapLimitConfig> cfgFileManager;

  public MapLimitManager(File configFile, File backupDir, ClusterController cc, FileSystem fs) throws AdminException {
    this.cc = cc;
    this.cfgFileManager = new ConfigFileManager<MapLimitConfig>(configFile, backupDir, fs) {
      @Override
      protected MapLimitConfig newConfigFile() {
        return new MapLimitConfig();
      }
    };

    try {
      cc.lockMapLimits(false);
      cfgFileManager.reset();
    } finally {
      cc.unlockMapLimits();
    }
  }

  public MapLimitSettings getSettings() {
    return new MapLimitSettings(cfgFileManager.getConfig().getSettings());
  }

  public void updateSettings(MapLimitSettings settings) throws AdminException {
    cfgFileManager.getConfig().setSettings(new MapLimitSettings(settings));

    try {
      cc.lockMapLimits(true);
      cfgFileManager.apply();
    } finally {
      cc.unlockMapLimits();
    }

    cc.applyMapLimits();
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    ConfigState state = cc.getMapLimitConfigState();
    long lastUpdate = cfgFileManager.getConfigFile().lastModified();
    Map<Integer, SmscConfigurationStatus> result = new HashMap<Integer, SmscConfigurationStatus>();
    for (Map.Entry<Integer, Long> e : state.getInstancesUpdateTimes().entrySet()) {
      SmscConfigurationStatus s = e.getValue() >= lastUpdate ? SmscConfigurationStatus.UP_TO_DATE : SmscConfigurationStatus.OUT_OF_DATE;
      result.put(e.getKey(), s);
    }
    return result;
  }
}

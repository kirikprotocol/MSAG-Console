package ru.novosoft.smsc.admin.route;

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
public class RouteSubjectManager implements SmscConfiguration {

  private final ConfigFileManager<RouteSubjectSettings> cfgFileManager;
  private final ClusterController cc;

  public RouteSubjectManager(File configFile, File backupDir, FileSystem fs, ClusterController cc) {
    this.cfgFileManager = new ConfigFileManager<RouteSubjectSettings>(configFile, backupDir, fs, new RoutesConfig());
    this.cc = cc;
  }

  public RouteSubjectSettings getSettings() throws AdminException {
    try {
      if (cc.isOnline())
        cc.lockRoutes(false);

      return cfgFileManager.load();
    } finally {
      if (cc.isOnline())
        cc.unlockRoutes();
    }
  }

  public void updateSettings(RouteSubjectSettings settings) throws AdminException {
    try {
      if (cc.isOnline())
        cc.lockRoutes(false);

      cfgFileManager.save(settings);

      if (cc.isOnline())
        cc.applyRoutes();
    } finally {
      if (cc.isOnline())
        cc.unlockRoutes();
    }
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    if (!cc.isOnline())
      return null;
    ConfigState state = cc.getRoutesState();
    long lastUpdate = cfgFileManager.getLastModified();
    Map<Integer, SmscConfigurationStatus> result = new HashMap<Integer, SmscConfigurationStatus>();
    for (Map.Entry<Integer, Long> e : state.getInstancesUpdateTimes().entrySet()) {
      SmscConfigurationStatus s = e.getValue() >= lastUpdate ? SmscConfigurationStatus.UP_TO_DATE : SmscConfigurationStatus.OUT_OF_DATE;
      result.put(e.getKey(), s);
    }
    return result;
  }
}

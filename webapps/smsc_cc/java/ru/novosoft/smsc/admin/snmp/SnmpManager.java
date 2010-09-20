package ru.novosoft.smsc.admin.snmp;

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
 * Менеджер, управляющий конфигурацией Snmp
 *
 * @author Artem Snopkov
 */
public class SnmpManager implements SmscConfiguration {

  private final ClusterController cc;
  private final ConfigFileManager<SnmpSettings> cfgFileManager;

  public SnmpManager(File configFile, File backupDir, ClusterController cc, FileSystem fileSystem) throws AdminException {
    this.cfgFileManager = new ConfigFileManager<SnmpSettings>(configFile, backupDir, fileSystem, new SnmpConfigFile());
    this.cc = cc;
  }

  public SnmpSettings getSettings() throws AdminException {
    try {
      if (cc.isOnline())
        cc.lockSnmp(false);
      return this.cfgFileManager.load();
    } finally {
      if (cc.isOnline())
        cc.unlockSnmp();
    }
  }

  public void updateSettings(SnmpSettings settings) throws AdminException {
    try {
      if (cc.isOnline())
        cc.lockSnmp(true);
      cfgFileManager.save(settings);
    } finally {
      if (cc.isOnline())
        cc.unlockSnmp();
    }

    if (cc.isOnline())
      cc.applySnmp();
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    if (!cc.isOnline())
      return null;

    ConfigState state = cc.getSnmpConfigState();
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

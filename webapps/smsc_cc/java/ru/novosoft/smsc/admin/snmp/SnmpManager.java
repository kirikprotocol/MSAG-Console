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
  private SnmpSettings currentSettings;

  public SnmpManager(File configFile, File backupDir, ClusterController cc, FileSystem fileSystem) throws AdminException {
    this.cfgFileManager = new ConfigFileManager<SnmpSettings>(configFile, backupDir, fileSystem, new SnmpConfigFile());

    try {
      cc.lockSnmp(false);
      currentSettings = this.cfgFileManager.load();
    } finally {
      cc.unlockSnmp();
    }

    this.cc = cc;
  }

  public SnmpSettings getSettings() {
    return new SnmpSettings(currentSettings);
  }

  public void updateSettings(SnmpSettings settings) throws AdminException {
    try {
      cc.lockSnmp(true);
      cfgFileManager.save(settings);
    } finally {
      cc.unlockSnmp();
    }

    currentSettings = new SnmpSettings(settings);

    cc.applySnmp();
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    ConfigState state = cc.getSnmpConfigState();
    long lastUpdate = cfgFileManager.getLastModified();
    Map<Integer, SmscConfigurationStatus> result = new HashMap<Integer, SmscConfigurationStatus>();
    for (Map.Entry<Integer, Long> e : state.getInstancesUpdateTimes().entrySet()) {
      SmscConfigurationStatus s = e.getValue() >= lastUpdate ? SmscConfigurationStatus.UP_TO_DATE : SmscConfigurationStatus.OUT_OF_DATE;
      result.put(e.getKey(), s);
    }
    return result;
  }
}

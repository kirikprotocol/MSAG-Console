package ru.novosoft.smsc.admin.smsc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ConfigState;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.config.SmscConfiguration;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.ServiceManager;

import java.io.File;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
@SuppressWarnings({"EmptyCatchBlock"})
public class SmscManager implements SmscConfiguration {

  private static final String SERVICE_ID="SMSC";

  private final ClusterController cc;
  private final ConfigFileManager<SmscConfigFile> cfgFileManager;
  private final ServiceManager serviceManager;

  public SmscManager(ServiceManager serviceManager, ClusterController cc, FileSystem fileSystem) throws AdminException {
    this.cc = cc;
    this.serviceManager = serviceManager;
    ServiceInfo info = getInfo(0);

    File configDir = new File(info.getBaseDir(), "conf");

    this.cfgFileManager = createManager(new File(configDir, "config.xml"), new File(configDir, "backup"), fileSystem);
  }

  SmscManager(File configFile, File backupDir, FileSystem fs, ClusterController cc, ServiceManager serviceManager) throws AdminException {
    this.cc = cc;
    this.serviceManager = serviceManager;
    this.cfgFileManager = createManager(configFile, backupDir, fs);
  }

  private ConfigFileManager<SmscConfigFile> createManager(File configFile, File backupDir, FileSystem fs) throws AdminException {
    ConfigFileManager<SmscConfigFile> cfgFileManager = new ConfigFileManager<SmscConfigFile>(configFile, backupDir, fs) {
      @Override
      protected SmscConfigFile newConfigFile() {
        return new SmscConfigFile();
      }
    };

    try {
      cc.lockMainConfig(false);
      cfgFileManager.reset();
    } finally {
      cc.unlockMainConfig();
    }

    return cfgFileManager;
  }

  private ServiceInfo getInfo(int instanceNumber) throws AdminException {
    ServiceInfo si = serviceManager.getService(SERVICE_ID + instanceNumber);
    if (si == null)
      throw new SmscException("cluster_controller_offline");
    return si;
  }

  public SmscSettings getSettings() {
    return new SmscSettings(cfgFileManager.getConfig().getSettings());
  }

  public void updateSettings(SmscSettings s) throws AdminException {
    cfgFileManager.getConfig().setSettings(new SmscSettings(s));

    try {
      cc.lockMainConfig(true);
      cfgFileManager.apply();
    } finally {
      cc.unlockMainConfig();
    }
  }

  public File getConfigDir() {
    return cfgFileManager.getConfigFile().getParentFile();
  }

  public File getConfigBackupDir() {
    return new File(getConfigDir(), "backup");
  }

  public int getSmscInstancesCount() {
    return cfgFileManager.getConfig().getSettings().getSmscInstancesCount();
  }

  public void startSmsc(int instanceNumber) throws AdminException {
    serviceManager.startService(SERVICE_ID + instanceNumber);
  }

  public void stopSmsc(int instanceNumber) throws AdminException {
    serviceManager.stopService(SERVICE_ID + instanceNumber);
  }

  public void switchSmsc(int instanceNumber, String toHost) throws AdminException {
    serviceManager.swichService(SERVICE_ID + instanceNumber, toHost);
  }

  public String getSmscOnlineHost(int instanceNumber) throws AdminException {
    return getInfo(instanceNumber).getOnlineHost();
  }

  public List<String> getSmscHosts(int instanceNumber) throws AdminException {
    return getInfo(instanceNumber).getHosts();
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    ConfigState state = cc.getMainConfigState();
    long lastUpdate = cfgFileManager.getConfigFile().lastModified();
    Map<Integer, SmscConfigurationStatus> result = new HashMap<Integer, SmscConfigurationStatus>();
    for (Map.Entry<Integer, Long> e : state.getInstancesUpdateTimes().entrySet()) {
      SmscConfigurationStatus s = e.getValue() >= lastUpdate ? SmscConfigurationStatus.UP_TO_DATE : SmscConfigurationStatus.OUT_OF_DATE;
      result.put(e.getKey(), s);
    }
    return result;
  }
}

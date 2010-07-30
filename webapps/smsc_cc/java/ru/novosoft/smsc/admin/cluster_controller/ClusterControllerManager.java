package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.ServiceManager;

import java.io.File;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class ClusterControllerManager  {

  public static final String SERVICE_ID = "ClusterController";

  private final ConfigFileManager<ClusterControllerConfig> cfgFileManager;
  private final ServiceManager serviceManager;

  public ClusterControllerManager(ServiceManager serviceManager, FileSystem fileSystem) throws AdminException {
    this.serviceManager = serviceManager;
    ServiceInfo info = getInfo();
    File conf = new File(info.getBaseDir(), "conf");

    this.cfgFileManager = createManager(new File(conf, "config.xml"), new File(conf, "backup"), fileSystem);
    this.cfgFileManager.reset();
  }

  ClusterControllerManager(File configFile, File backupDir, FileSystem fs, ServiceManager serviceManager) throws AdminException {
    this.serviceManager = serviceManager;
    this.cfgFileManager = createManager(configFile, backupDir, fs);
    this.cfgFileManager.reset();
  }

  private ConfigFileManager<ClusterControllerConfig> createManager(File configFile, File backupDir, FileSystem fs) {
    return new ConfigFileManager<ClusterControllerConfig>(configFile, backupDir, fs) {
      @Override
      protected ClusterControllerConfig newConfigFile() {
        return new ClusterControllerConfig();
      }
    };
  }

  private ServiceInfo getInfo() throws AdminException {
    ServiceInfo si = serviceManager.getService(SERVICE_ID);
    if (si == null)
      throw new ClusterControllerException("cluster_controller_offline");
    return si;
  }

  public ClusterControllerSettings getSettings() {
    return new ClusterControllerSettings(cfgFileManager.getConfig().getSettings());
  }

  public void updateSettings(ClusterControllerSettings newSettings) {
    cfgFileManager.getConfig().setSettings(new ClusterControllerSettings(newSettings));
  }

  public void startClusterController() throws AdminException {
    serviceManager.startService(SERVICE_ID);
  }

  public void stopClusterController() throws AdminException {
    serviceManager.stopService(SERVICE_ID);
  }

  public void switchClusterController(String toHost) throws AdminException {
    serviceManager.swichService(SERVICE_ID, toHost);
  }

  public String getControllerOnlineHost() throws AdminException {
    return getInfo().getOnlineHost();
  }

  public List<String> getControllerHosts() throws AdminException {
    return getInfo().getHosts();
  }
}

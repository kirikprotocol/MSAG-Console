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

  private final ConfigFileManager<ClusterControllerSettings> cfgFileManager;
  private final ServiceManager serviceManager;
  private ClusterControllerSettings currentSettings;

  public ClusterControllerManager(ServiceManager serviceManager, FileSystem fileSystem) throws AdminException {
    this.serviceManager = serviceManager;
    ServiceInfo info = getInfo();
    File conf = new File(info.getBaseDir(), "conf");

    this.cfgFileManager = new ConfigFileManager<ClusterControllerSettings>(new File(conf, "config.xml"), new File(conf, "backup"), fileSystem, new ClusterControllerConfig());
    this.currentSettings = this.cfgFileManager.load();
  }

  ClusterControllerManager(File configFile, File backupDir, FileSystem fs, ServiceManager serviceManager) throws AdminException {
    this.serviceManager = serviceManager;
    this.cfgFileManager = new ConfigFileManager<ClusterControllerSettings>(configFile, backupDir, fs, new ClusterControllerConfig());
    this.currentSettings = this.cfgFileManager.load();
  }

  private ServiceInfo getInfo() throws AdminException {
    ServiceInfo si = serviceManager.getService(SERVICE_ID);
    if (si == null)
      throw new ClusterControllerException("cluster_controller_offline");
    return si;
  }

  public ClusterControllerSettings getSettings() {
    return new ClusterControllerSettings(currentSettings);
  }

  public void updateSettings(ClusterControllerSettings newSettings) throws AdminException {
    cfgFileManager.save(newSettings);
    this.currentSettings = new ClusterControllerSettings(newSettings);
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

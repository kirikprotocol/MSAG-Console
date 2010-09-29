package ru.novosoft.smsc.admin.archive_daemon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.ConfigFileManager;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.ServiceManager;

import java.io.File;
import java.util.List;

/**
 * класс, управляющий настройками Archive Daemon-а
 *
 * @author Artem Snopkov
 */
public class ArchiveDaemonManagerImpl implements ArchiveDaemonManager {

  public static final String SERVICE_ID = "ArchiveDaemon";

  private final ConfigFileManager<ArchiveDaemonSettings> cfgFileManager;
  private final ServiceManager serviceManager;

  public ArchiveDaemonManagerImpl(ServiceManager serviceManager, FileSystem fs) throws AdminException {
    this.serviceManager = serviceManager;
    ServiceInfo info = getInfo();
    File archiveDaemonConf = new File(info.getBaseDir(), "conf");
    File archiveDaemonBackup = new File(archiveDaemonConf, "backup");
    this.cfgFileManager = new ConfigFileManager<ArchiveDaemonSettings>(new File(archiveDaemonConf, "config.xml"), archiveDaemonBackup, fs, new ArchiveDaemonConfig());
  }

  public static boolean isDaemonDeployed(ServiceManager serviceManager) throws AdminException {
    return serviceManager.getService(SERVICE_ID) != null;
  }

  public ArchiveDaemonSettings getSettings() throws AdminException {
    return cfgFileManager.load();
  }

  public void updateSettings(ArchiveDaemonSettings settings) throws AdminException {
    cfgFileManager.save(settings);
  }

  private ServiceInfo getInfo() throws AdminException {
    ServiceInfo info = serviceManager.getService(SERVICE_ID);
    if (info == null)
      throw new ArchiveDaemonException("archive_daemon_not_found");
    return info;
  }

  public String getDaemonOnlineHost() throws AdminException {
    return getInfo().getOnlineHost();
  }

  public void switchDaemon(String toHost) throws AdminException {
    serviceManager.swichService(SERVICE_ID, toHost);
  }

  public void startDaemon() throws AdminException {
    serviceManager.startService(SERVICE_ID);
  }

  public void stopDaemon() throws AdminException {
    serviceManager.stopService(SERVICE_ID);
  }

  public List<String> getDaemonHosts() throws AdminException {
    return getInfo().getHosts();
  }
}

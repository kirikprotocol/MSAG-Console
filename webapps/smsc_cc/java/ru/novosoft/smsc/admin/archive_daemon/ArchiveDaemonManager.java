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
public class ArchiveDaemonManager {

  public static final String SERVICE_ID = "ArchiveDaemon";

  private final ConfigFileManager<ArchiveDaemonConfig> cfgFileManager;
  private final ServiceManager serviceManager;

  public ArchiveDaemonManager(ServiceManager serviceManager, FileSystem fs) throws AdminException {
    this.serviceManager = serviceManager;
    ServiceInfo info = getInfo();
    File archiveDaemonConf = new File(info.getBaseDir(), "conf");
    File archiveDaemonBackup = new File(archiveDaemonConf, "backup");
    this.cfgFileManager = createManager(new File(archiveDaemonConf, "config.xml"), archiveDaemonBackup, fs);
    this.cfgFileManager.reset();
  }

  ArchiveDaemonManager(File configFile, File backupDir, FileSystem fs, ServiceManager serviceManager) throws AdminException {
    this.serviceManager = serviceManager;
    this.cfgFileManager = createManager(configFile, backupDir, fs);
    this.cfgFileManager.reset();
  }

  private ConfigFileManager<ArchiveDaemonConfig> createManager(File configFile, File backupDir, FileSystem fs) {
    return new ConfigFileManager<ArchiveDaemonConfig>(configFile, backupDir, fs) {
      @Override
      protected ArchiveDaemonConfig newConfigFile() {
        return new ArchiveDaemonConfig();
      }
    };
  }

  public static boolean isDaemonDeployed(ServiceManager serviceManager) throws AdminException {
    return serviceManager.getService(SERVICE_ID) != null;
  }

  /**
   * Возвращает текущие настройки ArchiveDaemon-а
   *
   * @return текущие настройки ArchiveDaemon-а
   */
  public ArchiveDaemonSettings getSettings() {
    ArchiveDaemonConfig config = cfgFileManager.getConfig();
    return new ArchiveDaemonSettings(config.getSettings());
  }

  /**
   * Обновляет настройки ArchiveDaemon-а.
   *
   * @param settings новый настройки ArchiveDaemon-а
   * @throws AdminException
   */
  public void updateSettings(ArchiveDaemonSettings settings) throws AdminException {
    ArchiveDaemonConfig config = cfgFileManager.getConfig();
    config.setSettings(new ArchiveDaemonSettings(settings));
    cfgFileManager.apply();
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

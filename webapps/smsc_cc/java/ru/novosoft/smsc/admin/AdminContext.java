package ru.novosoft.smsc.admin;

import ru.novosoft.smsc.admin.alias.AliasManager;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonConfig;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDemon;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.reschedule.RescheduleManager;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.ServiceManager;
import ru.novosoft.smsc.admin.smsc.SmscConfig;

import java.io.File;

/**
 * Основной класс для работы с конфигурацией СМСЦ. Доступ ко всем моделям должен осуществляться через
 * экземпляр данного класса.
 *
 * @author Artem Snopkov
 */
public class AdminContext {

  protected File appBaseDir;
  protected File servicesDir;

  protected ServiceManager serviceManager;
  protected FileSystem fileSystem;
  protected SmscConfig smscConfig;
  protected ArchiveDaemonConfig archiveDaemonConfig;
  protected ClusterController clusterController;
  protected AliasManager aliasManager;
  protected RescheduleManager rescheduleManager;
  protected InstallationType instType;

  protected AdminContext() {

  }

  public AdminContext(File appBaseDir, File initFile) throws AdminException {
    AdminContextConfig cfg = new AdminContextConfig(initFile);
    this.appBaseDir = appBaseDir;
    this.servicesDir = new File(appBaseDir, "services");
    this.instType = cfg.getInstallationType();

    switch (this.instType) {
      case SINGLE:
        serviceManager = ServiceManager.getServiceManagerForSingleInst(cfg.getSingleDaemonHost(), cfg.getSingleDaemonPort(), servicesDir);
        fileSystem = FileSystem.getFSForSingleInst();
        break;
      case HS:
        serviceManager = ServiceManager.getServiceManagerForHSInst(cfg.getHSDaemonHost(), cfg.getHSDaemonPort(), servicesDir, cfg.getHSDaemonHosts());
        fileSystem = FileSystem.getFSForHSInst(appBaseDir, cfg.getAppMirrorDirs());
        break;
      default:
        serviceManager = ServiceManager.getServiceManagerForHAInst(new File(appBaseDir, "conf/resourceGroups.properties"), servicesDir);
        fileSystem = FileSystem.getFSForHAInst();
    }

    ServiceInfo smscServiceInfo = serviceManager.getService("SMSC1");
    if (smscServiceInfo == null)
      throw new AdminContextException("service_not_found", "SMSC1");

    File smscConfigDir = new File(smscServiceInfo.getBaseDir(), "conf");
    if (!smscConfigDir.exists())
      throw new AdminContextException("dir_not_exists", smscConfigDir.getAbsolutePath());

    File smscConfigBackupDir = new File(smscConfigDir, "backup");

    smscConfig = new SmscConfig(new File(smscConfigDir, "config.xml"), smscConfigBackupDir, fileSystem);

    ServiceInfo archiveDaemonInfo = serviceManager.getService(ArchiveDemon.SERVICE_ID);
    archiveDaemonConfig = (archiveDaemonInfo == null) ? null : new ArchiveDaemonConfig(archiveDaemonInfo.getBaseDir(), fileSystem);

    clusterController = new ClusterController(serviceManager, fileSystem);

    aliasManager = new AliasManager(new File(smscConfigDir, "alias.bin"), clusterController, fileSystem);


  }

  public FileSystem getFileSystem() {
    return fileSystem;
  }

  public SmscConfig getSmscConfig() {
    return smscConfig;
  }

  public ArchiveDaemonConfig getArchiveDaemonConfig() {
    return archiveDaemonConfig;
  }

  public AliasManager getAliasManager() {
    return aliasManager;
  }

  public RescheduleManager getRescheduleManager() {
    return rescheduleManager;
  }

  public InstallationType getInstallationType() {
    return instType;
  }

  /**
   * Деинициализирует контекст.
   */
  public void shutdown() {
  }
}

package ru.novosoft.smsc.admin;

import ru.novosoft.smsc.admin.alias.AliasManager;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManager;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDemon;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroupManager;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ClusterControllerManager;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.fraud.FraudManager;
import ru.novosoft.smsc.admin.map_limit.MapLimitManager;
import ru.novosoft.smsc.admin.msc.MscManager;
import ru.novosoft.smsc.admin.reschedule.RescheduleManager;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.ServiceManager;
import ru.novosoft.smsc.admin.smsc.SmscManager;
import ru.novosoft.smsc.admin.snmp.SnmpManager;

import java.io.File;

/**
 * Фабрика для создания классов, позволяющих получить доступ к конфигурации и статистике.
 *
 * @author Artem Snopkov
 */
public class AdminContext {

  protected File appBaseDir;
  protected File servicesDir;

  protected ServiceManager serviceManager;
  protected FileSystem fileSystem;
  protected SmscManager smscManager;
  protected ArchiveDaemonManager archiveDaemonManager;
  protected ClusterController clusterController;
  protected AliasManager aliasManager;
  protected RescheduleManager rescheduleManager;
  protected FraudManager fraudManager;
  protected MapLimitManager mapLimitManager;
  protected SnmpManager snmpManager;
  protected ClosedGroupManager closedGroupManager;
  protected MscManager mscManager;
  protected InstallationType instType;
  protected ClusterControllerManager clusterControllerManager;

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

    ServiceInfo clusterControllerInfo = serviceManager.getService(ClusterController.SERVICE_ID);
    File clusterControllerConf = new File(clusterControllerInfo.getBaseDir(), "conf");

    clusterControllerManager = new ClusterControllerManager(new File(clusterControllerConf, "config.xml"), new File(clusterControllerConf, "backup"), fileSystem);

    clusterController = new ClusterController(clusterControllerManager, serviceManager);

    ServiceInfo smscServiceInfo = serviceManager.getService("SMSC1");
    if (smscServiceInfo == null)
      throw new AdminContextException("service_not_found", "SMSC1");

    File smscConfigDir = new File(smscServiceInfo.getBaseDir(), "conf");
    if (!smscConfigDir.exists())
      throw new AdminContextException("dir_not_exists", smscConfigDir.getAbsolutePath());

    File smscConfigBackupDir = new File(smscConfigDir, "backup");

    smscManager = new SmscManager(new File(smscConfigDir, "config.xml"), smscConfigBackupDir, clusterController, fileSystem);

    ServiceInfo archiveDaemonInfo = serviceManager.getService(ArchiveDemon.SERVICE_ID);
    File archiveDaemonConf = new File(archiveDaemonInfo.getBaseDir(), "conf");
    File archiveDaemonBackup = new File(archiveDaemonConf, "backup");
    archiveDaemonManager = (archiveDaemonInfo == null) ? null : new ArchiveDaemonManager(new File(archiveDaemonConf, "config.xml"), archiveDaemonBackup, fileSystem);

    aliasManager = new AliasManager(new File(smscConfigDir, "alias.bin"), clusterController, fileSystem);

    rescheduleManager = new RescheduleManager(new File(smscConfigDir, "schedule.xml"), smscConfigBackupDir, clusterController, fileSystem);

    fraudManager = new FraudManager(new File(smscConfigDir, "fraud.xml"), smscConfigBackupDir, clusterController, fileSystem);

    mapLimitManager = new MapLimitManager(new File(smscConfigDir, "maplimits.xml"), smscConfigBackupDir, clusterController, fileSystem);

    snmpManager = new SnmpManager(new File(smscConfigDir, "snmp.xml"), smscConfigBackupDir, clusterController, fileSystem);

    closedGroupManager = new ClosedGroupManager(new File(smscConfigDir, "snmp.xml"), smscConfigBackupDir, clusterController, fileSystem);

    mscManager = new MscManager(new File(smscManager.getCommonSettings().getMscStoreFile()), clusterController, fileSystem);
  }

  public FileSystem getFileSystem() {
    return fileSystem;
  }

  public SmscManager getSmscManager() {
    return smscManager;
  }

  public ArchiveDaemonManager getArchiveDaemonManager() {
    return archiveDaemonManager;
  }

  public AliasManager getAliasManager() {
    return aliasManager;
  }

  public RescheduleManager getRescheduleManager() {
    return rescheduleManager;
  }

  public FraudManager getFraudManager() {
    return fraudManager;
  }

  public MapLimitManager getMapLimitManager() {
    return mapLimitManager;
  }

  public SnmpManager getSnmpManager() {
    return snmpManager;
  }

  public ClosedGroupManager getClosedGroupManager() {
    return closedGroupManager;
  }

  public MscManager getMscManager() {
    return mscManager;
  }

  public InstallationType getInstallationType() {
    return instType;
  }

  /**
   * Деинициализирует контекст.
   */
  public void shutdown() {
    clusterController.shutdown();
  }
}

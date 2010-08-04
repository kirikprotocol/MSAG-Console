package ru.novosoft.smsc.admin;

import ru.novosoft.smsc.admin.alias.AliasManager;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManager;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroupManager;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ClusterControllerManager;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.fraud.FraudManager;
import ru.novosoft.smsc.admin.map_limit.MapLimitManager;
import ru.novosoft.smsc.admin.msc.MscManager;
import ru.novosoft.smsc.admin.reschedule.RescheduleManager;
import ru.novosoft.smsc.admin.resource.ResourceManager;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.ServiceManager;
import ru.novosoft.smsc.admin.sme.SmeManager;
import ru.novosoft.smsc.admin.smsc.SmscManager;
import ru.novosoft.smsc.admin.smsc.SmscSettings;
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
  protected SmeManager smeManager;
  protected ResourceManager resourceManager;

  protected AdminContext() {
    AdminContextLocator.registerContext(this);
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

    clusterControllerManager = new ClusterControllerManager(serviceManager, fileSystem);    
    clusterController = new ClusterController(clusterControllerManager);

    smscManager = new SmscManager(serviceManager, clusterController, fileSystem);

    File smscConfigDir = smscManager.getConfigDir();
    File smscConfigBackupDir = smscManager.getConfigBackupDir();

    if (ArchiveDaemonManager.isDaemonDeployed(serviceManager))
      archiveDaemonManager = new ArchiveDaemonManager(serviceManager, fileSystem);

    aliasManager = new AliasManager(new File(smscConfigDir, "alias.bin"), clusterController, fileSystem);

    rescheduleManager = new RescheduleManager(new File(smscConfigDir, "schedule.xml"), smscConfigBackupDir, clusterController, fileSystem);

    fraudManager = new FraudManager(new File(smscConfigDir, "fraud.xml"), smscConfigBackupDir, clusterController, fileSystem);    

    mapLimitManager = new MapLimitManager(new File(smscConfigDir, "maplimits.xml"), smscConfigBackupDir, clusterController, fileSystem);

    snmpManager = new SnmpManager(new File(smscConfigDir, "snmp.xml"), smscConfigBackupDir, clusterController, fileSystem);

    closedGroupManager = new ClosedGroupManager(new File(smscConfigDir, "snmp.xml"), smscConfigBackupDir, clusterController, fileSystem);

    SmscSettings s = smscManager.getSettings();

    mscManager = new MscManager(new File(s.getCommonSettings().getMscStoreFile()), clusterController, fileSystem);

    smeManager = new SmeManager(new File(smscConfigDir, "sme.xml"), smscConfigBackupDir, clusterController, serviceManager, fileSystem);

    resourceManager = new ResourceManager(smscConfigDir, smscConfigBackupDir, clusterController, fileSystem);

    AdminContextLocator.registerContext(this);
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

  public SmeManager getSmeManager() {
    return smeManager;
  }

  public ResourceManager getResourceManager() {
    return resourceManager;
  }

  public InstallationType getInstallationType() {
    return instType;
  }

  /**
   * Деинициализирует контекст.
   */
  public void shutdown() {
    clusterController.shutdown();
    AdminContextLocator.unregisterContext(this);
  }
}

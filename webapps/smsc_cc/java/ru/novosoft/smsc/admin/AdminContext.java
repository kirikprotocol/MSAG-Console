package ru.novosoft.smsc.admin;

import ru.novosoft.smsc.admin.acl.AclManager;
import ru.novosoft.smsc.admin.acl.AclManagerImpl;
import ru.novosoft.smsc.admin.alias.AliasManager;
import ru.novosoft.smsc.admin.alias.AliasManagerImpl;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManager;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManagerImpl;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDemon;
import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.admin.category.CategoryManagerImpl;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroupManager;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroupManagerImpl;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ClusterControllerManager;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.fraud.FraudManager;
import ru.novosoft.smsc.admin.fraud.FraudManagerImpl;
import ru.novosoft.smsc.admin.logging.LoggerManager;
import ru.novosoft.smsc.admin.logging.LoggerManagerImpl;
import ru.novosoft.smsc.admin.map_limit.MapLimitManager;
import ru.novosoft.smsc.admin.map_limit.MapLimitManagerImpl;
import ru.novosoft.smsc.admin.msc.MscManager;
import ru.novosoft.smsc.admin.msc.MscManagerImpl;
import ru.novosoft.smsc.admin.operative_store.OperativeStoreProvider;
import ru.novosoft.smsc.admin.profile.ProfileManager;
import ru.novosoft.smsc.admin.profile.ProfileManagerImpl;
import ru.novosoft.smsc.admin.provider.ProfiledManagerImpl;
import ru.novosoft.smsc.admin.provider.ProviderManager;
import ru.novosoft.smsc.admin.region.RegionManager;
import ru.novosoft.smsc.admin.region.RegionManagerImpl;
import ru.novosoft.smsc.admin.reschedule.RescheduleManager;
import ru.novosoft.smsc.admin.reschedule.RescheduleManagerImpl;
import ru.novosoft.smsc.admin.resource.ResourceManager;
import ru.novosoft.smsc.admin.resource.ResourceManagerImpl;
import ru.novosoft.smsc.admin.route.RouteSubjectManager;
import ru.novosoft.smsc.admin.route.RouteSubjectManagerImpl;
import ru.novosoft.smsc.admin.service.ServiceManager;
import ru.novosoft.smsc.admin.sme.SmeManager;
import ru.novosoft.smsc.admin.sme.SmeManagerImpl;
import ru.novosoft.smsc.admin.smsc.InstanceSettings;
import ru.novosoft.smsc.admin.smsc.SmscManager;
import ru.novosoft.smsc.admin.smsc.SmscManagerImpl;
import ru.novosoft.smsc.admin.smsc.SmscSettings;
import ru.novosoft.smsc.admin.snmp.SnmpManager;
import ru.novosoft.smsc.admin.snmp.SnmpManagerImpl;
import ru.novosoft.smsc.admin.timezone.TimezoneManager;
import ru.novosoft.smsc.admin.timezone.TimezoneManagerImpl;
import ru.novosoft.smsc.admin.users.UsersManager;
import ru.novosoft.smsc.admin.users.UsersManagerImpl;

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
  protected SmscManagerImpl smscManager;
  protected ArchiveDaemonManager archiveDaemonManager;
  protected ArchiveDemon archiveDaemon;
  protected ClusterController clusterController;
  protected AliasManager aliasManager;
  protected RescheduleManager rescheduleManager;
  protected UsersManager usersManager;
  protected FraudManager fraudManager;
  protected MapLimitManager mapLimitManager;
  protected SnmpManager snmpManager;
  protected ClosedGroupManager closedGroupManager;
  protected MscManager mscManager;
  protected InstallationType instType;
  protected ClusterControllerManager clusterControllerManager;
  protected SmeManager smeManager;
  protected ResourceManager resourceManager;
  protected ProviderManager providerManager;
  protected CategoryManager categoryManager;
  protected AclManager aclManager;
  protected RouteSubjectManager routeSubjectManager;
  protected TimezoneManager timezoneManager;
  protected RegionManager regionManager;
  protected ProfileManager profileManager;
  protected LoggerManager loggerManager;

  protected OperativeStoreProvider operativeStoreProvider;

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

    smscManager = new SmscManagerImpl(serviceManager, clusterController, fileSystem);

    File smscConfigDir = smscManager.getConfigDir();
    File smscConfigBackupDir = smscManager.getConfigBackupDir();

    if (ArchiveDaemonManagerImpl.isDaemonDeployed(serviceManager)) {
      archiveDaemonManager = new ArchiveDaemonManagerImpl(serviceManager, fileSystem);
      archiveDaemon = new ArchiveDemon(archiveDaemonManager);
    }

    aliasManager = new AliasManagerImpl(new File(smscManager.getSettings().getCommonSettings().getAliasStoreFile()), clusterController, fileSystem);

    rescheduleManager = new RescheduleManagerImpl(new File(smscConfigDir, "schedule.xml"), smscConfigBackupDir, clusterController, fileSystem);

    File usersFile = new File(cfg.getUsersFile());

    usersManager = new UsersManagerImpl(usersFile, new File(usersFile.getParentFile(), "backup"), fileSystem);

    fraudManager = new FraudManagerImpl(new File(smscConfigDir, "fraud.xml"), smscConfigBackupDir, clusterController, fileSystem);

    mapLimitManager = new MapLimitManagerImpl(new File(smscConfigDir, "maplimits.xml"), smscConfigBackupDir, clusterController, fileSystem);

    snmpManager = new SnmpManagerImpl(new File(smscConfigDir, "snmp.xml"), smscConfigBackupDir, clusterController, fileSystem);

    closedGroupManager = new ClosedGroupManagerImpl(new File(smscConfigDir, "ClosedGroups.xml"), smscConfigBackupDir, clusterController, fileSystem);

    SmscSettings s = smscManager.getSettings();

    mscManager = new MscManagerImpl(new File(s.getCommonSettings().getMscStoreFile()), clusterController, fileSystem);

    smeManager = new SmeManagerImpl(new File(smscConfigDir, "sme.xml"), smscConfigBackupDir, clusterController, serviceManager, fileSystem);

    resourceManager = new ResourceManagerImpl(smscConfigDir, smscConfigBackupDir, clusterController, fileSystem);

    providerManager = new ProfiledManagerImpl(initFile, smscConfigBackupDir, fileSystem);

    categoryManager = new CategoryManagerImpl(initFile, smscConfigBackupDir, fileSystem);

    aclManager = new AclManagerImpl(clusterController);

    routeSubjectManager = new RouteSubjectManagerImpl(new File(smscConfigDir, "routes.xml"), smscConfigBackupDir, fileSystem, clusterController);

    timezoneManager = new TimezoneManagerImpl(new File(smscConfigDir, "timezones.xml"), smscConfigBackupDir, fileSystem, clusterController);

    regionManager = new RegionManagerImpl(new File(smscConfigDir, "regions.xml"), smscConfigBackupDir, fileSystem);

    profileManager = new ProfileManagerImpl(AdminMode.smsx, new File(s.getCommonSettings().getProfilerStoreFile()), fileSystem, clusterController);

    loggerManager = new LoggerManagerImpl(clusterController);

    File[] operativeStorages = new File[s.getSmscInstancesCount()];
    for (int i=0;i<s.getSmscInstancesCount(); i++) {
      InstanceSettings is = s.getInstanceSettings(i);
      operativeStorages[i] = new File(is.getLocalStoreFilename());
    }

    operativeStoreProvider = new OperativeStoreProvider(operativeStorages, fileSystem);

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

  public UsersManager getUsersManager() {
    return usersManager;
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

  public ProviderManager getProviderManager() {
    return providerManager;
  }

  public CategoryManager getCategoryManager() {
    return categoryManager;
  }

  public AclManager getAclManager() {
    return aclManager;
  }

  public RouteSubjectManager getRouteSubjectManager() {
    return routeSubjectManager;
  }

  public TimezoneManager getTimezoneManager() {
    return timezoneManager;
  }

  public RegionManager getRegionManager() {
    return regionManager;
  }

  public ProfileManager getProfileManager() {
    return profileManager;
  }

  public LoggerManager getLoggerManager() {
    return loggerManager;
  }

  public OperativeStoreProvider getOperativeStoreProvider() {
    return operativeStoreProvider;
  }

  public ArchiveDemon getArchiveDaemon() {
    return archiveDaemon;
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

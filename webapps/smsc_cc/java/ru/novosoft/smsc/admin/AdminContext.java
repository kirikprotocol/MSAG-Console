package ru.novosoft.smsc.admin;

import ru.novosoft.smsc.admin.acl.AclManager;
import ru.novosoft.smsc.admin.acl.AclManagerImpl;
import ru.novosoft.smsc.admin.alias.AliasManager;
import ru.novosoft.smsc.admin.alias.AliasManagerImpl;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemon;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManager;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManagerImpl;
import ru.novosoft.smsc.admin.category.CategoryManager;
import ru.novosoft.smsc.admin.category.CategoryManagerImpl;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroupManager;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroupManagerImpl;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.cluster_controller.ClusterControllerManager;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.fraud.FraudManager;
import ru.novosoft.smsc.admin.fraud.FraudManagerImpl;
import ru.novosoft.smsc.admin.logging.LoggerManager;
import ru.novosoft.smsc.admin.logging.LoggerManagerImpl;
import ru.novosoft.smsc.admin.map_limit.MapLimitManager;
import ru.novosoft.smsc.admin.map_limit.MapLimitManagerImpl;
import ru.novosoft.smsc.admin.msc.MscManager;
import ru.novosoft.smsc.admin.msc.MscManagerImpl;
import ru.novosoft.smsc.admin.operative_store.OperativeStoreManager;
import ru.novosoft.smsc.admin.perfmon.PerfMonitorContext;
import ru.novosoft.smsc.admin.perfmon.PerfMonitorManager;
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
import ru.novosoft.smsc.admin.stat.SmscStatContext;
import ru.novosoft.smsc.admin.stat.SmscStatProvider;
import ru.novosoft.smsc.admin.timezone.TimezoneManager;
import ru.novosoft.smsc.admin.timezone.TimezoneManagerImpl;
import ru.novosoft.smsc.admin.topmon.TopMonitorContext;
import ru.novosoft.smsc.admin.topmon.TopMonitorManager;
import ru.novosoft.smsc.admin.users.UsersManager;
import ru.novosoft.smsc.admin.users.UsersManagerImpl;
import ru.novosoft.smsc.util.InetAddress;

import java.io.File;
import java.util.List;
import java.util.Map;

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
  protected ArchiveDaemon archiveDaemon;
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

  protected OperativeStoreManager operativeStoreManager;

  protected SmscStatProvider smscStatProvider;

  protected PerfMonitorManager perfMonitorManager;
  protected TopMonitorManager topMonitorManager;

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

    SmscManagerImpl _smscManager = new SmscManagerImpl(serviceManager, clusterController, fileSystem);

    File smscConfigDir = _smscManager.getConfigDir();
    File smscConfigBackupDir = _smscManager.getConfigBackupDir();

    if (ArchiveDaemonManagerImpl.isDaemonDeployed(serviceManager)) {
      archiveDaemonManager = new ArchiveDaemonManagerImpl(serviceManager, fileSystem);
      archiveDaemon = new ArchiveDaemon(archiveDaemonManager);
    }

    SmscSettings s = smscManager.getSettings();

    aliasManager = new AliasManagerImpl(new File(s.getCommonSettings().getAliasStoreFile()), clusterController, fileSystem);

    rescheduleManager = new RescheduleManagerImpl(new File(smscConfigDir, "schedule.xml"), smscConfigBackupDir, clusterController, fileSystem);

    File usersFile = new File(cfg.getUsersFile());

    usersManager = new UsersManagerImpl(usersFile, new File(usersFile.getParentFile(), "backup"), fileSystem);

    fraudManager = new FraudManagerImpl(new File(smscConfigDir, "fraud.xml"), smscConfigBackupDir, clusterController, fileSystem);

    mapLimitManager = new MapLimitManagerImpl(new File(smscConfigDir, "maplimits.xml"), smscConfigBackupDir, clusterController, fileSystem);

    snmpManager = new SnmpManagerImpl(new File(smscConfigDir, "snmp.xml"), smscConfigBackupDir,
        new File(s.getCommonSettings().getSnmpCsvFileDir()), clusterController, fileSystem);

    closedGroupManager = new ClosedGroupManagerImpl(new File(smscConfigDir, "ClosedGroups.xml"), smscConfigBackupDir, clusterController, fileSystem);

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

    perfMonitorManager = new PerfMonitorManager(new PerfMonitorContextImpl(cfg.isPerfMonSupport64Bit(), cfg.getPerfMonitorPorts(), _smscManager));
    topMonitorManager = new TopMonitorManager(new TopMonitorContextImpl(cfg.getTopMonitorPorts(), _smscManager));

    smscManager = new WSmscManagerImpl(_smscManager, perfMonitorManager, topMonitorManager);

    File[] operativeStorages = new File[s.getSmscInstancesCount()];
    for (int i=0;i<s.getSmscInstancesCount(); i++) {
      InstanceSettings is = s.getInstanceSettings(i);
      operativeStorages[i] = new File(is.getLocalStoreFilename());
    }

    operativeStoreManager = new OperativeStoreManager(operativeStorages, fileSystem, clusterController);

    smscStatProvider = new SmscStatProvider(new SmscStatContextImpl(smscManager, fileSystem));

    AdminContextLocator.registerContext(this);
  }

  public PerfMonitorManager getPerfMonitorManager() {
    return perfMonitorManager;
  }

  public TopMonitorManager getTopMonitorManager() {
    return topMonitorManager;
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

  public OperativeStoreManager getOperativeStoreManager() {
    return operativeStoreManager;
  }

  public ArchiveDaemon getArchiveDaemon() {
    return archiveDaemon;
  }

  public SmscStatProvider getSmscStatProvider() {
    return smscStatProvider;
  }

  /**
   * Деинициализирует контекст.
   */
  public void shutdown() {
    try{
      clusterController.shutdown();
    }catch (Exception e){
      e.printStackTrace();
    }
    if(perfMonitorManager != null) {
      try{
        perfMonitorManager.shutdown();
      }catch (Exception e){
        e.printStackTrace();
      }
    }
    if(topMonitorManager != null) {
      try{
        topMonitorManager.shutdown();
      }catch (Exception e){
        e.printStackTrace();
      }
    }
    AdminContextLocator.unregisterContext(this);
  }

  protected static class SmscStatContextImpl implements SmscStatContext {

    private final SmscManager smscManager;
    private final FileSystem fileSystem;

    public SmscStatContextImpl(SmscManager smscManager, FileSystem fileSystem) {
      this.smscManager = smscManager;
      this.fileSystem = fileSystem;
    }

    public File[] getStatDirs() throws AdminException {
      SmscSettings s = smscManager.getSettings();
      int countSmscs =  s.getSmscInstancesCount();
      File[] dirs = new File[countSmscs];
      for(int i=0; i < countSmscs; i++) {
        dirs[i] = new File(s.getInstanceSettings(i).getMessageStoreStatsDir());
      }
      return dirs;
    }

    public FileSystem getFileSystem() {
      return fileSystem;
    }
  }

  protected static class PerfMonitorContextImpl implements PerfMonitorContext {

    private final boolean support64Bit;
    private final int[] appletports;

    private final SmscManager smscManager;

    public PerfMonitorContextImpl(boolean support64Bit, int[] appletports, SmscManager smscManager) {
      this.support64Bit = support64Bit;
      this.appletports = appletports;
      this.smscManager = smscManager;
    }

    public InetAddress getPerfMonitorAddress(int instance) throws AdminException {
      InstanceSettings is = smscManager.getSettings().getInstanceSettings(instance);
      return new InetAddress(is.getCorePerfHost(), is.getCorePerfPort());
    }

    public int getPerfMonitorCount() throws AdminException {
      return appletports.length;
    }

    public int getAppletPort(int instance) throws AdminException {
      try{
        return appletports[instance];
      }catch (IndexOutOfBoundsException e) {
        return -1;
      }
    }

    public boolean isSupport64Bit() throws AdminException {
      return support64Bit;
    }
  }

  protected static class TopMonitorContextImpl implements TopMonitorContext {
    private final int[] appletports;

    private final SmscManager smscManager;

    public TopMonitorContextImpl(int[] appletports, SmscManager smscManager) {
      this.appletports = appletports;
      this.smscManager = smscManager;
    }

    public InetAddress getTopMonitorAddress(int instance) throws AdminException {
      InstanceSettings is = smscManager.getSettings().getInstanceSettings(instance);
      return new InetAddress(is.getSmePerfHost(), is.getSmePerfPort());
    }

    public int getTopMonitorCount() throws AdminException {
      return appletports.length;
    }

    public int getAppletPort(int instance) throws AdminException {
      try{
        return appletports[instance];
      }catch (IndexOutOfBoundsException e) {
        return -1;
      }
    }
  }

  protected static class WSmscManagerImpl implements SmscManager{
    private final TopMonitorManager topMonitorManager;
    private final PerfMonitorManager perfMonitorManager;
    private final SmscManager smscManager;

    public WSmscManagerImpl(SmscManager smscManager, PerfMonitorManager perfMonitorManager, TopMonitorManager topMonitorManager) throws AdminException {
      this.smscManager = smscManager;
      this.perfMonitorManager = perfMonitorManager;
      this.topMonitorManager = topMonitorManager;
    }


    public void updateSettings(SmscSettings s) throws AdminException {
      SmscSettings old = getSettings();
      boolean topRestart = false;
      boolean perfRestart = false;
      for(int i=0; i<s.getSmscInstancesCount(); i++) {
        InstanceSettings n = s.getInstanceSettings(i);
        InstanceSettings o = old.getInstanceSettings(i);
        if(n.getCorePerfHost() != null) {
          if(!n.getCorePerfHost().equals(o.getCorePerfHost())) {
            perfRestart = true;
          }
        }else if(o.getCorePerfHost() != null) {
          perfRestart = true;
        }
        if(n.getSmePerfHost() != null) {
          if(!n.getSmePerfHost().equals(o.getSmePerfHost())) {
            topRestart = true;
          }
        }else if(o.getSmePerfHost() != null) {
          topRestart = true;
        }
        if(o.getCorePerfPort() != n.getCorePerfPort()) {
          perfRestart = true;
        }
        if(o.getSmePerfPort() != n.getSmePerfPort()) {
          topRestart = true;
        }
      }
      smscManager.updateSettings(s);
      if(topRestart) {
        topMonitorManager.updateTopServers();
      }
      if(perfRestart) {
        perfMonitorManager.updatePerfServers();
      }
    }

    public void startSmsc(int instanceNumber) throws AdminException {
      smscManager.startSmsc(instanceNumber);
    }

    public void stopSmsc(int instanceNumber) throws AdminException {
      smscManager.stopSmsc(instanceNumber);
    }

    public void switchSmsc(int instanceNumber, String toHost) throws AdminException {
      smscManager.switchSmsc(instanceNumber, toHost);
    }

    public String getSmscOnlineHost(int instanceNumber) throws AdminException {
      return smscManager.getSmscOnlineHost(instanceNumber);
    }

    public List<String> getSmscHosts(int instanceNumber) throws AdminException {
      return smscManager.getSmscHosts(instanceNumber);
    }

    public SmscSettings getSettings() throws AdminException {
      return smscManager.getSettings();
    }

    public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
      return smscManager.getStatusForSmscs();
    }
  }
}
package ru.novosoft.smsc.admin;

import ru.novosoft.smsc.admin.acl.TestAclManager;
import ru.novosoft.smsc.admin.alias.AliasManagerImplTest;
import ru.novosoft.smsc.admin.alias.TestAliasManager;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonConfigTest;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManagerImpl;
import ru.novosoft.smsc.admin.archive_daemon.TestArchiveDaemon;
import ru.novosoft.smsc.admin.archive_daemon.TestArchiveDaemonManager;
import ru.novosoft.smsc.admin.category.TestCategoryManager;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroupManagerImplTest;
import ru.novosoft.smsc.admin.closed_groups.TestClosedGroupManager;
import ru.novosoft.smsc.admin.cluster_controller.ClusterControllerConfigTest;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterController;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterControllerManager;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.filesystem.TestFileSystem;
import ru.novosoft.smsc.admin.fraud.FraudManagerImplTest;
import ru.novosoft.smsc.admin.fraud.TestFraudManager;
import ru.novosoft.smsc.admin.logging.TestLoggerManager;
import ru.novosoft.smsc.admin.map_limit.MapLimitManagerImplTest;
import ru.novosoft.smsc.admin.map_limit.TestMapLimitManager;
import ru.novosoft.smsc.admin.mcisme.MCISmeManagerImpl;
import ru.novosoft.smsc.admin.mcisme.TestMCISmeManager;
import ru.novosoft.smsc.admin.msc.MscManagerImplTest;
import ru.novosoft.smsc.admin.msc.TestMscManager;
import ru.novosoft.smsc.admin.operative_store.OperativeStoreManager;
import ru.novosoft.smsc.admin.operative_store.OperativeStoreProviderTest;
import ru.novosoft.smsc.admin.perfmon.TestPerfMonitorManager;
import ru.novosoft.smsc.admin.profile.TestProfileManager;
import ru.novosoft.smsc.admin.provider.TestProviderManager;
import ru.novosoft.smsc.admin.region.RegionsConfigTest;
import ru.novosoft.smsc.admin.region.TestRegionManager;
import ru.novosoft.smsc.admin.reschedule.RescheduleManagerImplTest;
import ru.novosoft.smsc.admin.reschedule.TestRescheduleManager;
import ru.novosoft.smsc.admin.resource.ResourceFileTest;
import ru.novosoft.smsc.admin.resource.TestResourceManager;
import ru.novosoft.smsc.admin.route.Route;
import ru.novosoft.smsc.admin.route.RoutesConfigTest;
import ru.novosoft.smsc.admin.route.TestRouteSubjectManager;
import ru.novosoft.smsc.admin.service.TestServiceManagerHA;
import ru.novosoft.smsc.admin.service.TestServiceManagerSingle;
import ru.novosoft.smsc.admin.sme.SmeConfigFileTest;
import ru.novosoft.smsc.admin.sme.TestSmeManager;
import ru.novosoft.smsc.admin.smsc.SmscManagerImpl;
import ru.novosoft.smsc.admin.smsc.SmscManagerImplTest;
import ru.novosoft.smsc.admin.smsc.SmscSettings;
import ru.novosoft.smsc.admin.smsc.TestSmscManager;
import ru.novosoft.smsc.admin.snmp.SnmpManagerImplTest;
import ru.novosoft.smsc.admin.snmp.TestSnmpManager;
import ru.novosoft.smsc.admin.stat.SmscStatContext;
import ru.novosoft.smsc.admin.stat.TestSmscStatProvider;
import ru.novosoft.smsc.admin.timezone.TestTimezoneManager;
import ru.novosoft.smsc.admin.timezone.TimezonesConfigTest;
import ru.novosoft.smsc.admin.topmon.TestTopMonitorManager;
import ru.novosoft.smsc.admin.users.TestUsersManager;
import ru.novosoft.smsc.admin.users.UsersManagerImplTest;
import testutils.TestUtils;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class TestAdminContext extends AdminContext {

  private static final int PROFILES_VERSION=AdminMode.smsx ? 0x00010100 : 0x00010000;

  private void prepareServices(File servicesDir, AdminContextConfig cfg, FileSystem fs) throws IOException, AdminException {
    File ccDir = new File(servicesDir, "ClusterController/conf");
    fs.mkdirs(ccDir);
    TestUtils.exportResource(ClusterControllerConfigTest.class.getResourceAsStream("config.xml"), new File(ccDir, "config.xml"), false, fs);

    File adDir = new File(servicesDir, "ArchiveDaemon/conf");
    fs.mkdirs(adDir);
    TestUtils.exportResource(ArchiveDaemonConfigTest.class.getResourceAsStream("daemon.xml"), new File(adDir, "daemon.xml"), false, fs);


    File mciSmeDir = new File(servicesDir, "MCISme/conf");
    fs.mkdirs(mciSmeDir);
    TestUtils.exportResource(TestMCISmeManager.class.getResourceAsStream("config.xml"), new File(mciSmeDir, "config.xml"), false, fs);

    File smscDir = new File(servicesDir, "SMSC1/conf");
    fs.mkdirs(smscDir);
    TestUtils.exportResource(SmscManagerImplTest.class.getResourceAsStream("config.xml"), new File(smscDir, "config.xml"), false, fs);
    TestUtils.exportResource(AliasManagerImplTest.class.getResourceAsStream("aliases.bin"), new File(smscDir, "aliases.bin"), false, fs);
    TestUtils.exportResource(ClosedGroupManagerImplTest.class.getResourceAsStream("ClosedGroups.xml"), new File(smscDir, "ClosedGroups.xml"), false, fs);
    TestUtils.exportResource(FraudManagerImplTest.class.getResourceAsStream("fraud.xml"), new File(smscDir, "fraud.xml"), false, fs);
    TestUtils.exportResource(MapLimitManagerImplTest.class.getResourceAsStream("maplimits.xml"), new File(smscDir, "maplimits.xml"), false, fs);
    TestUtils.exportResource(MscManagerImplTest.class.getResourceAsStream("msc.bin"), new File(smscDir, "msc.bin"), false, fs);
    TestUtils.exportResource(RescheduleManagerImplTest.class.getResourceAsStream("schedule.xml"), new File(smscDir, "schedule.xml"), false, fs);
    File usersFile = new File(cfg.getUsersFile());
    fs.mkdirs(usersFile.getParentFile());
    TestUtils.exportResource(UsersManagerImplTest.class.getResourceAsStream("users.xml"), usersFile, false, fs);
    TestUtils.exportResource(SnmpManagerImplTest.class.getResourceAsStream("snmp.xml"), new File(smscDir, "snmp.xml"), false, fs);
    TestUtils.exportResource(SmeConfigFileTest.class.getResourceAsStream("sme.xml"), new File(smscDir, "sme.xml"), false, fs);
    TestUtils.exportResource(ResourceFileTest.class.getResourceAsStream("resources_en_en.xml"), new File(smscDir, "resources_en_en.xml"), false, fs);
    TestUtils.exportResource(ResourceFileTest.class.getResourceAsStream("resources_ru_ru.xml"), new File(smscDir, "resources_ru_ru.xml"), false, fs);
    TestUtils.exportResource(RoutesConfigTest.class.getResourceAsStream("routes.xml"), new File(smscDir, "routes.xml"), false, fs);
    TestUtils.exportResource(TimezonesConfigTest.class.getResourceAsStream("timezones.xml"), new File(smscDir, "timezones.xml"), false, fs);
    TestUtils.exportResource(RegionsConfigTest.class.getResourceAsStream("regions.xml"), new File(smscDir, "regions.xml"), false, fs);
    TestUtils.exportResource(TestProfileManager.emptyProfilesFileAsStream(AdminMode.smsx, PROFILES_VERSION), new File(smscDir, "profiles.bin"), false, fs);

//    for(int i=1; i<3;i++) {
      File operStoreDir = new File(servicesDir, "SMSC"+1+File.separatorChar+"store"+File.separatorChar+"operative");
      fs.mkdirs(operStoreDir);
      TestUtils.exportResource(OperativeStoreProviderTest.class.getResourceAsStream("store.bin"), new File(operStoreDir, "store.bin"), false, fs);
      TestUtils.exportResource(OperativeStoreProviderTest.class.getResourceAsStream("store.20111010113952.bin"), new File(operStoreDir, "store.20111010113952.bin"), false, fs);
//    }

    SimpleDateFormat format = new SimpleDateFormat("yyyy-MM");
    File statDir =  new File(servicesDir, "SMSC1"+File.separatorChar+"store"+File.separatorChar+"stat"+File.separatorChar+format.format(new Date()));
    fs.mkdirs(statDir);
    TestUtils.exportResource(TestSmscStatProvider.class.getResourceAsStream("05.rts"), new File(statDir, "05.rts"), false, fs);
    TestUtils.exportResource(TestSmscStatProvider.class.getResourceAsStream("06.rts"), new File(statDir, "06.rts"), false, fs);
    TestUtils.exportResource(TestSmscStatProvider.class.getResourceAsStream("07.rts"), new File(statDir, "07.rts"), false, fs);


    File snmp = new File(servicesDir, "snmp");
    if(!snmp.exists()) {
      fs.mkdirs(snmp);
    }
    TestUtils.exportResource(SnmpManagerImplTest.class.getResourceAsStream("20101109_220719.ucs.csv"), new File(snmp, "20101109_220719.ucs.csv"), false, fs);

  }

  public TestAdminContext(File appBaseDir, File initFile, int smscInstancesNumber) throws AdminException {
    this(appBaseDir, initFile, smscInstancesNumber, new TestFileSystem());
  }

  public TestAdminContext(File appBaseDir, File initFile, int smscInstancesNumber, FileSystem fs) throws AdminException {
    fileSystem = fs;
    if (!fs.exists(initFile)) {
      fs.mkdirs(initFile.getParentFile());
      try {
        TestUtils.exportResource(new FileInputStream(initFile), initFile, fs);
      } catch (IOException e) {
        e.printStackTrace();
      }
    }

    AdminContextConfig cfg = new AdminContextConfig(initFile);
    this.appBaseDir = appBaseDir;
    this.servicesDir = new File(appBaseDir, "services");
    this.instType = cfg.getInstallationType();

    if (!servicesDir.exists())
      fileSystem.mkdirs(servicesDir);

    try {
      prepareServices(servicesDir, cfg, fileSystem);
    } catch (IOException e) {
      throw new AdminContextException("Can't create services dir!");
    }

    File smscDir = new File(servicesDir, "SMSC1/conf");

    if (instType == InstallationType.SINGLE)
      serviceManager = new TestServiceManagerSingle(servicesDir, smscInstancesNumber);
    else
      serviceManager = new TestServiceManagerHA(servicesDir, smscInstancesNumber, new String[] {"host0", "host1"});



    clusterController = new TestClusterController(
        new File(smscDir, "aliases.bin"),
        new File(smscDir, "msc.bin"),
        new File(smscDir, "profiles.bin"), AdminMode.smsx, PROFILES_VERSION,
        fileSystem, smscInstancesNumber);

    clusterControllerManager = new TestClusterControllerManager(serviceManager, fileSystem);

    SmscManagerImpl _smscManager = new TestSmscManager(serviceManager, clusterController, fileSystem);

    File smscConfigDir = _smscManager.getConfigDir();
    File smscConfigBackupDir = _smscManager.getConfigBackupDir();

    aliasManager = new TestAliasManager(new File(smscConfigDir, "aliases.bin"), clusterController, fileSystem);

    rescheduleManager = new TestRescheduleManager(new File(smscConfigDir, "schedule.xml"), smscConfigBackupDir, clusterController, fileSystem);

    File usersFile = new File(cfg.getUsersFile());

    usersManager = new TestUsersManager(usersFile, new File(usersFile.getParentFile(), "backup"), fileSystem);

    fraudManager = new TestFraudManager(new File(smscConfigDir, "fraud.xml"), smscConfigBackupDir, clusterController, fileSystem);

    mapLimitManager = new TestMapLimitManager(new File(smscConfigDir, "maplimits.xml"), smscConfigBackupDir, clusterController, fileSystem);

    SmscSettings s = _smscManager.getSettings();

    snmpManager = new TestSnmpManager(new File(smscConfigDir, "snmp.xml"), smscConfigBackupDir,
        new File(servicesDir, "snmp"), clusterController, fileSystem);

    closedGroupManager = new TestClosedGroupManager(new File(smscConfigDir, "ClosedGroups.xml"), smscConfigBackupDir, clusterController, fileSystem);

    mscManager = new TestMscManager(new File(s.getCommonSettings().getMscStoreFile()), clusterController, fileSystem);

    smeManager = new TestSmeManager(new File(smscConfigDir, "sme.xml"), smscConfigBackupDir, clusterController, serviceManager, fileSystem);

    resourceManager = new TestResourceManager(smscConfigDir, smscConfigBackupDir, clusterController, fileSystem);

    providerManager = new TestProviderManager(initFile, smscConfigBackupDir, fileSystem);

    categoryManager = new TestCategoryManager(initFile, smscConfigBackupDir, fileSystem);

    aclManager = new TestAclManager(clusterController);

    routeSubjectManager = new TestRouteSubjectManager(new File(smscConfigDir, "routes.xml"), smscConfigBackupDir, fileSystem, clusterController);

    if (ArchiveDaemonManagerImpl.isDaemonDeployed(serviceManager)) {
      archiveDaemonManager = new TestArchiveDaemonManager(serviceManager, fileSystem, cfg.getArchiveExportSettings());
      List<String> routes = new LinkedList<String>();
      List<String> smes = new LinkedList<String>();
      for(Route r : routeSubjectManager.getSettings().getRoutes()) {
        routes.add(r.getName());
      }
      smes.addAll(smeManager.getSmesSmscStatuses().keySet());

      archiveDaemon = new TestArchiveDaemon(archiveDaemonManager, routes, smes);
    }

    if(MCISmeManagerImpl.isSmeDeployed(serviceManager)) {
//      mciSmeManager = new MCISmeManagerImpl(serviceManager, fileSystem);
//      mciSme = new MCISme("",0);//todo
//      mmlConsole = new MMLConsole(new Properties(), mciSme); //todo
    }

    timezoneManager = new TestTimezoneManager(new File(smscConfigDir, "timezones.xml"), smscConfigBackupDir, fileSystem, clusterController);

    regionManager = new TestRegionManager(new File(smscConfigDir, "regions.xml"), smscConfigBackupDir, fileSystem);

    profileManager = new TestProfileManager(AdminMode.smsx, new File(smscConfigDir, "profiles.bin"), fileSystem, clusterController);

    loggerManager = new TestLoggerManager(clusterController);

    File[] operStores = new File[smscInstancesNumber];
    operStores[0] = new File(servicesDir, "SMSC1"+File.separatorChar+"store"+File.separatorChar+"operative"+File.separatorChar+"store.bin");
    for (int i =0; i<smscInstancesNumber; i++)
      operStores[i] = new File(servicesDir, "SMSC" + i +File.separatorChar+"store"+File.separatorChar+"operative"+File.separatorChar+"store.bin");

    operativeStoreManager = new OperativeStoreManager(operStores, fileSystem, clusterController);
    operativeExportSettings = cfg.getOperExportSettings();

    final File[] statsDir = new File[1];
    statsDir[0] = new File(servicesDir, "SMSC1"+File.separatorChar+"store"+File.separatorChar+"stat");
    smscStatProvider = new TestSmscStatProvider(new SmscStatContext() {
      public File[] getStatDirs() {
        return statsDir;
      }

      public FileSystem getFileSystem() {
        return fileSystem;
      }
    });
    statExportSettings = cfg.getStatExportSettings();
    perfMonitorManager = new TestPerfMonitorManager(cfg.getPerfMonitorPorts());
    topMonitorManager = new TestTopMonitorManager(cfg.getTopMonitorPorts());

    smscManager = new WSmscManagerImpl(_smscManager, perfMonitorManager, topMonitorManager);
  }

  public TestAdminContext() throws AdminException {
    this(new File("."), new File("test", "webconfig.xml"), 3);
  }

  public TestAdminContext(File appBaseDir, File initFile) throws AdminException {
    this(appBaseDir, initFile, 3);
  }


}

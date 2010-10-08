package ru.novosoft.smsc.admin;

import ru.novosoft.smsc.admin.acl.TestAclManager;
import ru.novosoft.smsc.admin.alias.AliasManagerImplTest;
import ru.novosoft.smsc.admin.alias.TestAliasManager;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonConfigTest;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManagerImpl;
import ru.novosoft.smsc.admin.archive_daemon.TestArchiveDaemonManager;
import ru.novosoft.smsc.admin.category.TestCategoryManager;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroupManagerImplTest;
import ru.novosoft.smsc.admin.closed_groups.TestClosedGroupManager;
import ru.novosoft.smsc.admin.cluster_controller.ClusterControllerConfigTest;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterController;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterControllerManager;
import ru.novosoft.smsc.admin.filesystem.TestFileSystem;
import ru.novosoft.smsc.admin.fraud.FraudManagerImplTest;
import ru.novosoft.smsc.admin.fraud.TestFraudManager;
import ru.novosoft.smsc.admin.logging.TestLoggerManager;
import ru.novosoft.smsc.admin.map_limit.MapLimitManagerImplTest;
import ru.novosoft.smsc.admin.map_limit.TestMapLimitManager;
import ru.novosoft.smsc.admin.msc.MscManagerImplTest;
import ru.novosoft.smsc.admin.msc.TestMscManager;
import ru.novosoft.smsc.admin.operative_store.OperativeStoreProvider;
import ru.novosoft.smsc.admin.operative_store.OperativeStoreProviderTest;
import ru.novosoft.smsc.admin.profile.TestProfileManager;
import ru.novosoft.smsc.admin.provider.TestProviderManager;
import ru.novosoft.smsc.admin.region.RegionsConfigTest;
import ru.novosoft.smsc.admin.region.TestRegionManager;
import ru.novosoft.smsc.admin.reschedule.RescheduleManagerImplTest;
import ru.novosoft.smsc.admin.reschedule.TestRescheduleManager;
import ru.novosoft.smsc.admin.resource.ResourceFileTest;
import ru.novosoft.smsc.admin.resource.TestResourceManager;
import ru.novosoft.smsc.admin.route.RoutesConfigTest;
import ru.novosoft.smsc.admin.route.TestRouteSubjectManager;
import ru.novosoft.smsc.admin.service.TestServiceManagerHA;
import ru.novosoft.smsc.admin.service.TestServiceManagerSingle;
import ru.novosoft.smsc.admin.sme.SmeConfigFileTest;
import ru.novosoft.smsc.admin.sme.TestSmeManager;
import ru.novosoft.smsc.admin.smsc.SmscManagerImplTest;
import ru.novosoft.smsc.admin.smsc.SmscSettings;
import ru.novosoft.smsc.admin.smsc.TestSmscManager;
import ru.novosoft.smsc.admin.snmp.SnmpManagerImplTest;
import ru.novosoft.smsc.admin.snmp.TestSnmpManager;
import ru.novosoft.smsc.admin.timezone.TestTimezoneManager;
import ru.novosoft.smsc.admin.timezone.TimezonesConfigTest;
import ru.novosoft.smsc.admin.users.TestUsersManager;
import ru.novosoft.smsc.admin.users.UsersManagerImplTest;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;

/**
 * @author Artem Snopkov
 */
public class TestAdminContext extends AdminContext {

  private static final int PROFILES_VERSION=AdminMode.smsx ? 0x00010100 : 0x00010000;

  private void prepareServices(File servicesDir, AdminContextConfig cfg) throws IOException, AdminException {
    File ccDir = new File(servicesDir, "ClusterController/conf");
    ccDir.mkdirs();
    TestUtils.exportResource(ClusterControllerConfigTest.class.getResourceAsStream("config.xml"), new File(ccDir, "config.xml"), false);

    File adDir = new File(servicesDir, "ArchiveDaemon/conf");
    adDir.mkdirs();
    TestUtils.exportResource(ArchiveDaemonConfigTest.class.getResourceAsStream("config.xml"), new File(adDir, "config.xml"), false);

    File smscDir = new File(servicesDir, "SMSC1/conf");
    smscDir.mkdirs();
    TestUtils.exportResource(SmscManagerImplTest.class.getResourceAsStream("config.xml"), new File(smscDir, "config.xml"), false);
    TestUtils.exportResource(AliasManagerImplTest.class.getResourceAsStream("aliases.bin"), new File(smscDir, "aliases.bin"), false);
    TestUtils.exportResource(ClosedGroupManagerImplTest.class.getResourceAsStream("ClosedGroups.xml"), new File(smscDir, "ClosedGroups.xml"), false);
    TestUtils.exportResource(FraudManagerImplTest.class.getResourceAsStream("fraud.xml"), new File(smscDir, "fraud.xml"), false);
    TestUtils.exportResource(MapLimitManagerImplTest.class.getResourceAsStream("maplimits.xml"), new File(smscDir, "maplimits.xml"), false);
    TestUtils.exportResource(MscManagerImplTest.class.getResourceAsStream("msc.bin"), new File(smscDir, "msc.bin"), false);
    TestUtils.exportResource(RescheduleManagerImplTest.class.getResourceAsStream("schedule.xml"), new File(smscDir, "schedule.xml"), false);
    File usersFile = new File(cfg.getUsersFile());
    TestUtils.exportResource(UsersManagerImplTest.class.getResourceAsStream("users.xml"), usersFile, false);
    TestUtils.exportResource(SnmpManagerImplTest.class.getResourceAsStream("snmp.xml"), new File(smscDir, "snmp.xml"), false);
    TestUtils.exportResource(SmeConfigFileTest.class.getResourceAsStream("sme.xml"), new File(smscDir, "sme.xml"), false);
    TestUtils.exportResource(ResourceFileTest.class.getResourceAsStream("resources_en_en.xml"), new File(smscDir, "resources_en_en.xml"), false);
    TestUtils.exportResource(ResourceFileTest.class.getResourceAsStream("resources_ru_ru.xml"), new File(smscDir, "resources_ru_ru.xml"), false);
    TestUtils.exportResource(RoutesConfigTest.class.getResourceAsStream("routes.xml"), new File(smscDir, "routes.xml"), false);
    TestUtils.exportResource(TimezonesConfigTest.class.getResourceAsStream("timezones.xml"), new File(smscDir, "timezones.xml"), false);
    TestUtils.exportResource(RegionsConfigTest.class.getResourceAsStream("regions.xml"), new File(smscDir, "regions.xml"), false);
    TestUtils.exportResource(TestProfileManager.emptyProfilesFileAsStream(AdminMode.smsx, PROFILES_VERSION), new File(smscDir, "profiles.bin"), false);

    File operStoreDir = new File(servicesDir, "SMSC1/store/operative");
    operStoreDir.mkdirs();
    TestUtils.exportResource(OperativeStoreProviderTest.class.getResourceAsStream("store.bin"), new File(operStoreDir, "store.bin"), false);
  }

  public TestAdminContext(File appBaseDir, File initFile, int smscInstancesNumber) throws AdminException {
    AdminContextConfig cfg = new AdminContextConfig(initFile);
    this.appBaseDir = appBaseDir;
    this.servicesDir = new File(appBaseDir, "services");
    this.instType = cfg.getInstallationType();

    if (!servicesDir.exists())
      servicesDir.mkdirs();

    try {
      prepareServices(servicesDir, cfg);
    } catch (IOException e) {
      throw new AdminContextException("Can't create services dir!");
    }

    File smscDir = new File(servicesDir, "SMSC1/conf");

    if (instType == InstallationType.SINGLE)
      serviceManager = new TestServiceManagerSingle(servicesDir, smscInstancesNumber);
    else
      serviceManager = new TestServiceManagerHA(servicesDir, smscInstancesNumber, new String[] {"host0", "host1", "host2"});

    fileSystem = new TestFileSystem();

    clusterController = new TestClusterController(
        new File(smscDir, "aliases.bin"),
        new File(smscDir, "msc.bin"),
        new File(smscDir, "profiles.bin"), AdminMode.smsx, PROFILES_VERSION,
        fileSystem, smscInstancesNumber);

    clusterControllerManager = new TestClusterControllerManager(serviceManager, fileSystem);

    smscManager = new TestSmscManager(serviceManager, clusterController, fileSystem);

    File smscConfigDir = smscManager.getConfigDir();
    File smscConfigBackupDir = smscManager.getConfigBackupDir();

    if (ArchiveDaemonManagerImpl.isDaemonDeployed(serviceManager))
      archiveDaemonManager = new TestArchiveDaemonManager(serviceManager, fileSystem);

    aliasManager = new TestAliasManager(new File(smscConfigDir, "aliases.bin"), clusterController, fileSystem);

    rescheduleManager = new TestRescheduleManager(new File(smscConfigDir, "schedule.xml"), smscConfigBackupDir, clusterController, fileSystem);

    File usersFile = new File(cfg.getUsersFile());

    usersManager = new TestUsersManager(usersFile, new File(usersFile.getParentFile(), "backup"), fileSystem);

    fraudManager = new TestFraudManager(new File(smscConfigDir, "fraud.xml"), smscConfigBackupDir, clusterController, fileSystem);

    mapLimitManager = new TestMapLimitManager(new File(smscConfigDir, "maplimits.xml"), smscConfigBackupDir, clusterController, fileSystem);

    snmpManager = new TestSnmpManager(new File(smscConfigDir, "snmp.xml"), smscConfigBackupDir, clusterController, fileSystem);

    closedGroupManager = new TestClosedGroupManager(new File(smscConfigDir, "ClosedGroups.xml"), smscConfigBackupDir, clusterController, fileSystem);

    SmscSettings s = smscManager.getSettings();

    mscManager = new TestMscManager(new File(s.getCommonSettings().getMscStoreFile()), clusterController, fileSystem);

    smeManager = new TestSmeManager(new File(smscConfigDir, "sme.xml"), smscConfigBackupDir, clusterController, serviceManager, fileSystem);

    resourceManager = new TestResourceManager(smscConfigDir, smscConfigBackupDir, clusterController, fileSystem);

    providerManager = new TestProviderManager(initFile, smscConfigBackupDir, fileSystem);

    categoryManager = new TestCategoryManager(initFile, smscConfigBackupDir, fileSystem);

    aclManager = new TestAclManager(clusterController);

    routeSubjectManager = new TestRouteSubjectManager(new File(smscConfigDir, "routes.xml"), smscConfigBackupDir, fileSystem, clusterController);

    timezoneManager = new TestTimezoneManager(new File(smscConfigDir, "timezones.xml"), smscConfigBackupDir, fileSystem, clusterController);

    regionManager = new TestRegionManager(new File(smscConfigDir, "regions.xml"), smscConfigBackupDir, fileSystem);

    profileManager = new TestProfileManager(AdminMode.smsx, new File(smscConfigDir, "profiles.bin"), fileSystem, clusterController);

    loggerManager = new TestLoggerManager(clusterController);

    File[] operStores = new File[smscInstancesNumber];
    operStores[0] = new File(servicesDir, "SMSC1/store/operative/store.bin");
    for (int i =0; i<smscInstancesNumber; i++)
      operStores[i] = new File(servicesDir, "SMSC" + i + "/store/operative/store.bin");

    operativeStoreProvider = new OperativeStoreProvider(operStores, fileSystem);    
  }

  public TestAdminContext() throws AdminException {
    this(new File("."), new File("test", "webconfig.xml"), 3);
  }

  public TestAdminContext(File appBaseDir, File initFile) throws AdminException {
    this(appBaseDir, initFile, 3);
  }


}

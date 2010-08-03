package ru.novosoft.smsc.admin;

import ru.novosoft.smsc.admin.alias.AliasManagerTest;
import ru.novosoft.smsc.admin.alias.TestAliasManager;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonConfigTest;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonManager;
import ru.novosoft.smsc.admin.archive_daemon.TestArchiveDaemonManager;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroupManagerTest;
import ru.novosoft.smsc.admin.closed_groups.TestClosedGroupManager;
import ru.novosoft.smsc.admin.cluster_controller.ClusterControllerConfigTest;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterController;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterControllerManager;
import ru.novosoft.smsc.admin.filesystem.TestFileSystem;
import ru.novosoft.smsc.admin.fraud.FraudManagerTest;
import ru.novosoft.smsc.admin.fraud.TestFraudManager;
import ru.novosoft.smsc.admin.map_limit.MapLimitManagerTest;
import ru.novosoft.smsc.admin.map_limit.TestMapLimitManager;
import ru.novosoft.smsc.admin.msc.MscManagerTest;
import ru.novosoft.smsc.admin.msc.TestMscManager;
import ru.novosoft.smsc.admin.reschedule.RescheduleManagerTest;
import ru.novosoft.smsc.admin.reschedule.TestRescheduleManager;
import ru.novosoft.smsc.admin.service.TestServiceManagerSingle;
import ru.novosoft.smsc.admin.sme.SmeConfigFileTest;
import ru.novosoft.smsc.admin.sme.TestSmeManager;
import ru.novosoft.smsc.admin.smsc.SmscManagerTest;
import ru.novosoft.smsc.admin.smsc.SmscSettings;
import ru.novosoft.smsc.admin.smsc.TestSmscManager;
import ru.novosoft.smsc.admin.snmp.SnmpManagerTest;
import ru.novosoft.smsc.admin.snmp.TestSnmpManager;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;

/**
 * @author Artem Snopkov
 */
public class TestAdminContext extends AdminContext {

  private void prepareServices(File servicesDir) throws IOException {
    File ccDir = new File(servicesDir, "ClusterController/conf");
    ccDir.mkdirs();
    TestUtils.exportResource(ClusterControllerConfigTest.class.getResourceAsStream("config.xml"), new File(ccDir, "config.xml"));

    File adDir = new File(servicesDir, "ArchiveDaemon/conf");
    adDir.mkdirs();
    TestUtils.exportResource(ArchiveDaemonConfigTest.class.getResourceAsStream("config.xml"), new File(adDir, "config.xml"));

    File smscDir = new File(servicesDir, "SMSC0/conf");
    smscDir.mkdirs();
    TestUtils.exportResource(SmscManagerTest.class.getResourceAsStream("config.xml"), new File(smscDir, "config.xml"));
    TestUtils.exportResource(AliasManagerTest.class.getResourceAsStream("aliases.bin"), new File(smscDir, "aliases.bin"));
    TestUtils.exportResource(ClosedGroupManagerTest.class.getResourceAsStream("ClosedGroups.xml"), new File(smscDir, "ClosedGroup.xml"));
    TestUtils.exportResource(FraudManagerTest.class.getResourceAsStream("fraud.xml"), new File(smscDir, "fraud.xml"));
    TestUtils.exportResource(MapLimitManagerTest.class.getResourceAsStream("maplimits.xml"), new File(smscDir, "maplimits.xml"));
    TestUtils.exportResource(MscManagerTest.class.getResourceAsStream("msc.bin"), new File(smscDir, "msc.bin"));
    TestUtils.exportResource(RescheduleManagerTest.class.getResourceAsStream("schedule.xml"), new File(smscDir, "schedule.xml"));
    TestUtils.exportResource(SnmpManagerTest.class.getResourceAsStream("snmp.xml"), new File(smscDir, "snmp.xml"));
    TestUtils.exportResource(SmeConfigFileTest.class.getResourceAsStream("sme.xml"), new File(smscDir, "sme.xml"));
  }

  public TestAdminContext(File appBaseDir, File initFile, int smscInstancesNumber) throws AdminException {
    AdminContextConfig cfg = new AdminContextConfig(initFile);
    this.appBaseDir = appBaseDir;
    this.servicesDir = new File(appBaseDir, "services");
    this.instType = cfg.getInstallationType();

    if (!servicesDir.exists())
      servicesDir.mkdirs();

    try {
      prepareServices(servicesDir);
    } catch (IOException e) {
      throw new AdminContextException("Can't create services dir!");
    }

    File smscDir = new File(servicesDir, "SMSC0/conf");

    serviceManager = new TestServiceManagerSingle(servicesDir, smscInstancesNumber);
    clusterController = new TestClusterController(new File(smscDir, "aliases.bin"), new File(smscDir, "msc.bin"), smscInstancesNumber);

    fileSystem = new TestFileSystem();

    clusterControllerManager = new TestClusterControllerManager(serviceManager, fileSystem);
    
    smscManager = new TestSmscManager(serviceManager, clusterController, fileSystem);

    File smscConfigDir = smscManager.getConfigDir();
    File smscConfigBackupDir = smscManager.getConfigBackupDir();

    if (ArchiveDaemonManager.isDaemonDeployed(serviceManager))
      archiveDaemonManager = new TestArchiveDaemonManager(serviceManager, fileSystem);

    aliasManager = new TestAliasManager(new File(smscConfigDir, "alias.bin"), clusterController, fileSystem);

    rescheduleManager = new TestRescheduleManager(new File(smscConfigDir, "schedule.xml"), smscConfigBackupDir, clusterController, fileSystem);

    fraudManager = new TestFraudManager(new File(smscConfigDir, "fraud.xml"), smscConfigBackupDir, clusterController, fileSystem);

    mapLimitManager = new TestMapLimitManager(new File(smscConfigDir, "maplimits.xml"), smscConfigBackupDir, clusterController, fileSystem);

    snmpManager = new TestSnmpManager(new File(smscConfigDir, "snmp.xml"), smscConfigBackupDir, clusterController, fileSystem);

    closedGroupManager = new TestClosedGroupManager(new File(smscConfigDir, "snmp.xml"), smscConfigBackupDir, clusterController, fileSystem);

    SmscSettings s = smscManager.getSettings();

    mscManager = new TestMscManager(new File(s.getCommonSettings().getMscStoreFile()), clusterController, fileSystem);

    smeManager = new TestSmeManager(new File(smscConfigDir, "sme.xml"), smscConfigBackupDir, clusterController, serviceManager, fileSystem);
  }

  public TestAdminContext() throws AdminException {
    this(new File("."), new File("test", "webconfig.xml"), 2);
  }

  public TestAdminContext(File appBaseDir, File initFile) throws AdminException {
    this(appBaseDir, initFile, 2);
  }


}

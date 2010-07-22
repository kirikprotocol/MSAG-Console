package ru.novosoft.smsc.admin;

import ru.novosoft.smsc.admin.alias.TestAliasManager;
import ru.novosoft.smsc.admin.archive_daemon.TestArchiveDaemonManager;
import ru.novosoft.smsc.admin.closed_groups.TestClosedGroupManager;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterController;
import ru.novosoft.smsc.admin.fraud.TestFraudManager;
import ru.novosoft.smsc.admin.map_limit.TestMapLimitManager;
import ru.novosoft.smsc.admin.msc.TestMscManager;
import ru.novosoft.smsc.admin.reschedule.TestRescheduleManager;
import ru.novosoft.smsc.admin.smsc.TestSmscManager;
import ru.novosoft.smsc.admin.snmp.TestSnmpManager;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class TestAdminContext extends AdminContext {

  public TestAdminContext() throws AdminException {
    this(new File("."), new File("test", "webconfig.xml"));
  }

  public TestAdminContext(File appBaseDir, File initFile) throws AdminException {
    AdminContextConfig cfg = new AdminContextConfig(initFile);
    this.appBaseDir = appBaseDir;
    this.servicesDir = new File(appBaseDir, "services");
    this.instType = cfg.getInstallationType();

    TestClusterController cc = new TestClusterController();
    smscManager = new TestSmscManager();
    archiveDaemonManager = new TestArchiveDaemonManager();
    aliasManager = new TestAliasManager();
    rescheduleManager = new TestRescheduleManager(cc);
    fraudManager = new TestFraudManager(cc);
    mapLimitManager = new TestMapLimitManager(cc);
    snmpManager = new TestSnmpManager(cc);
    closedGroupManager = new TestClosedGroupManager(cc);
    mscManager = new TestMscManager(cc);
  }




}

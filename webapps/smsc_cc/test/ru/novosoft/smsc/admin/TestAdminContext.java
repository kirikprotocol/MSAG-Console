package ru.novosoft.smsc.admin;

import ru.novosoft.smsc.admin.alias.TestAliasManager;
import ru.novosoft.smsc.admin.archive_daemon.TestArchiveDaemonManager;
import ru.novosoft.smsc.admin.fraud.TestFraudManager;
import ru.novosoft.smsc.admin.map_limit.TestMapLimitManager;
import ru.novosoft.smsc.admin.reschedule.TestRescheduleManager;
import ru.novosoft.smsc.admin.smsc.TestSmscManager;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class TestAdminContext extends AdminContext {

  public TestAdminContext(File appBaseDir, File initFile) throws AdminException {
    AdminContextConfig cfg = new AdminContextConfig(initFile);
    this.appBaseDir = appBaseDir;
    this.servicesDir = new File(appBaseDir, "services");
    this.instType = cfg.getInstallationType();

    smscManager = new TestSmscManager();
    archiveDaemonManager = new TestArchiveDaemonManager();
    aliasManager = new TestAliasManager();
    rescheduleManager = new TestRescheduleManager();
    fraudManager = new TestFraudManager();
    mapLimitManager = new TestMapLimitManager();
  }




}

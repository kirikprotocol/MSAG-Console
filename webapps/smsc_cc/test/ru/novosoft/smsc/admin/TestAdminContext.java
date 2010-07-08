package ru.novosoft.smsc.admin;

import ru.novosoft.smsc.admin.alias.TestAliasManager;
import ru.novosoft.smsc.admin.archive_daemon.TestArchiveDaemonConfig;
import ru.novosoft.smsc.admin.reschedule.TestRescheduleManager;
import ru.novosoft.smsc.admin.smsc.TestSmscConfig;

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

    smscConfig = new TestSmscConfig();
    archiveDaemonConfig = new TestArchiveDaemonConfig();
    aliasManager = new TestAliasManager();
    rescheduleManager = new TestRescheduleManager();
  }




}

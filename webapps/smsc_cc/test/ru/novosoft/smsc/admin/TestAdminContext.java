package ru.novosoft.smsc.admin;

import ru.novosoft.smsc.admin.alias.AliasManager;
import ru.novosoft.smsc.admin.alias.TestAliasManager;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonConfig;
import ru.novosoft.smsc.admin.archive_daemon.TestArchiveDaemonConfig;
import ru.novosoft.smsc.admin.smsc.SmscConfig;
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
  }

  public SmscConfig getSmscConfig() {
    return smscConfig;
  }

  public ArchiveDaemonConfig getArchiveDaemonConfig() {
    return archiveDaemonConfig;
  }

  public AliasManager getAliasManager() {
    return aliasManager;
  }

  public InstallationType getInstallationType() {
    return instType;
  }


}

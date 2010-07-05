package ru.novosoft.smsc.admin;

import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonConfig;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDemon;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.ServiceManager;
import ru.novosoft.smsc.admin.smsc_config.SmscConfig;

import java.io.File;

/**
 * @author Artem Snopkov
 */
public class AdminContext {

  private final AdminContextConfig cfg;
  private final File appBaseDir;
  private final File servicesDir;

  private final ServiceManager serviceManager;
  private final FileSystem fileSystem;
  private final SmscConfig smscConfig;
  private final ArchiveDaemonConfig archiveDaemonConfig;

  public AdminContext(File appBaseDir, File initFile) throws AdminException {
    this.cfg = new AdminContextConfig(initFile);
    this.appBaseDir = appBaseDir;
    this.servicesDir = new File(appBaseDir, "services");

    switch (cfg.getInstallationType()) {
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

    ServiceInfo smscServiceInfo = serviceManager.getService("SMSC1");
    if (smscServiceInfo == null)
      throw new AdminException("Service SMSC1 does not exist");

    smscConfig = new SmscConfig(smscServiceInfo.getBaseDir(), fileSystem);

    ServiceInfo archiveDaemonInfo = serviceManager.getService(ArchiveDemon.SERVICE_ID);
    archiveDaemonConfig = (archiveDaemonInfo == null) ? null : new ArchiveDaemonConfig(archiveDaemonInfo.getBaseDir(), fileSystem);
    
  }



  public SmscConfig getSmscConfig() {
    return smscConfig;
  }

  public ArchiveDaemonConfig getArchiveDaemonConfig() {
    return archiveDaemonConfig;
  }
}

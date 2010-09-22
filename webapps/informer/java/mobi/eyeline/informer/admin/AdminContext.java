package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.filesystem.FileSystem;

import java.io.File;

/**
 * Фабрика для создания классов, позволяющих получить доступ к конфигурации и статистике.
 *
 * @author Artem Snopkov
 */
public class AdminContext {

  protected File appBaseDir;
//  protected File servicesDir;

//  protected ServiceManager serviceManager;
  protected FileSystem fileSystem;
  protected InstallationType instType;

  protected AdminContext() {
  }

  public AdminContext(File appBaseDir, InstallationType type, File[] appMirrorDirs) throws AdminException {
    this.appBaseDir = appBaseDir;
//    this.servicesDir = new File(appBaseDir, "services");
    this.instType = type;

    switch (this.instType) {
      case SINGLE:
//        serviceManager = ServiceManager.getServiceManagerForSingleInst(cfg.getSingleDaemonHost(), cfg.getSingleDaemonPort(), servicesDir);
        fileSystem = FileSystem.getFSForSingleInst();
        break;
      case HS:
//        serviceManager = ServiceManager.getServiceManagerForHSInst(cfg.getHSDaemonHost(), cfg.getHSDaemonPort(), servicesDir, cfg.getHSDaemonHosts());
        fileSystem = FileSystem.getFSForHSInst(appBaseDir, appMirrorDirs);
        break;
      default:
//      serviceManager = ServiceManager.getServiceManagerForHAInst(new File(appBaseDir, "conf/resourceGroups.properties"), servicesDir);
        fileSystem = FileSystem.getFSForHAInst();
    }


  }

  public FileSystem getFileSystem() {
    return fileSystem;
  }

  public InstallationType getInstallationType() {
    return instType;
  }
}

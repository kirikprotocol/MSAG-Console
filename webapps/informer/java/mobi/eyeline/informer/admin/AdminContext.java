package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.users.UsersManager;

import java.io.File;

/**
 * Фабрика для создания классов, позволяющих получить доступ к конфигурации и статистике.
 *
 * @author Artem Snopkov
 */
public class AdminContext {

  protected File appBaseDir;
  protected File servicesDir;

//  protected ServiceManager serviceManager;
  protected FileSystem fileSystem;
  protected UsersManager usersManager;
  protected InstallationType instType;

  protected AdminContext() {
  }

  public AdminContext(File appBaseDir, File initFile) throws AdminException {
    AdminContextConfig cfg = new AdminContextConfig(initFile);
    this.appBaseDir = appBaseDir;
//    this.servicesDir = new File(appBaseDir, "services");
    this.instType = cfg.getInstallationType();

    switch (this.instType) {
      case SINGLE:
//        serviceManager = ServiceManager.getServiceManagerForSingleInst(cfg.getSingleDaemonHost(), cfg.getSingleDaemonPort(), servicesDir);
        fileSystem = FileSystem.getFSForSingleInst();
        break;
      case HS:
//        serviceManager = ServiceManager.getServiceManagerForHSInst(cfg.getHSDaemonHost(), cfg.getHSDaemonPort(), servicesDir, cfg.getHSDaemonHosts());
        fileSystem = FileSystem.getFSForHSInst(appBaseDir, cfg.getAppMirrorDirs());
        break;
      default:
//        serviceManager = ServiceManager.getServiceManagerForHAInst(new File(appBaseDir, "conf/resourceGroups.properties"), servicesDir);
        fileSystem = FileSystem.getFSForHAInst();
    }

    File usersFile = new File(cfg.getUsersFile());

    usersManager = new UsersManager(usersFile, new File(usersFile.getParentFile(), "backup"), fileSystem);
  }

  public FileSystem getFileSystem() {
    return fileSystem;
  }

  public InstallationType getInstallationType() {
    return instType;
  }

  public UsersManager getUsersManager() {
    return usersManager;
  }

}

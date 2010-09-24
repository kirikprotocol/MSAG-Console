package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.informer.InformerConfigManager;
import mobi.eyeline.informer.admin.informer.InformerSettings;
import mobi.eyeline.informer.admin.journal.Journal;
import mobi.eyeline.informer.admin.users.UsersManager;
import mobi.eyeline.informer.admin.users.UsersSettings;
import mobi.eyeline.informer.web.WebConfig;

import java.io.File;

/**
 * Класс для управления моделью
 *
 * @author Aleksandr Khalitov
 */
public class AdminContext {

  protected File appBaseDir;

  protected FileSystem fileSystem;

  protected InstallationType instType;

  protected Journal journal;

  protected UsersManager usersManager;

  protected WebConfig webConfig;

  protected InformerConfigManager informerConfigManager;

  protected AdminContext() {
  }

  public AdminContext(File appBaseDir, WebConfig webConfig) throws AdminException{
    this.webConfig = webConfig;
    this.appBaseDir = appBaseDir;

    this.instType = webConfig.getInstallationType();

    switch (this.instType) {
      case SINGLE:
        fileSystem = FileSystem.getFSForSingleInst();
        break;
      case HS:
        fileSystem = FileSystem.getFSForHSInst(appBaseDir, webConfig.getAppMirrorDirs());
        break;
      default:
        fileSystem = FileSystem.getFSForHAInst();
    }
    File usersFile = new File(webConfig.getUsersFile());
    journal = new Journal(new File(webConfig.getJournalDir()), fileSystem);
    informerConfigManager = new InformerConfigManager(new File(appBaseDir,"conf"+File.separatorChar+"config.xml"),
        new File(appBaseDir,"conf"+File.separatorChar+"backup"), fileSystem);
    usersManager = new UsersManager(usersFile, new File(usersFile.getParentFile(), "backup"), fileSystem);

  }

  public Journal getJournal() {
    return journal;
  }

  public FileSystem getFileSystem() {
    return fileSystem;
  }

  public InstallationType getInstallationType() {
    return instType;
  }

  public UsersSettings getUsersSettings() throws AdminException {
    return usersManager.getUsersSettings();
  }

  public void updateUserSettings(UsersSettings usersSettings) throws AdminException {
    usersManager.updateSettings(usersSettings);
  }

  public InformerSettings getConfigSettings() throws AdminException {
    return informerConfigManager.getConfigSettings();
  }

  public void updateConfigSettings(InformerSettings informerSettings) throws AdminException {
    informerConfigManager.updateSettings(informerSettings);
  }
}

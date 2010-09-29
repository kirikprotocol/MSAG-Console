package mobi.eyeline.informer.admin;

import com.eyelinecom.whoisd.personalization.PersonalizationClientPool;
import com.eyelinecom.whoisd.personalization.exceptions.PersonalizationClientException;
import mobi.eyeline.informer.admin.blacklist.BlackListManagerImpl;
import mobi.eyeline.informer.admin.blacklist.BlacklistManager;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.informer.InformerConfigManager;
import mobi.eyeline.informer.admin.informer.InformerSettings;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.infosme.protogen.InfosmeImpl;
import mobi.eyeline.informer.admin.journal.Journal;
import mobi.eyeline.informer.admin.users.UsersManager;
import mobi.eyeline.informer.admin.users.UsersSettings;

import java.io.File;
import java.util.Collection;
import java.util.Properties;

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

  protected Infosme infosme;

  protected BlacklistManager blacklistManager;

  protected PersonalizationClientPool personalizationClientPool;

  protected AdminContext() {
  }

  public AdminContext(File appBaseDir, WebConfig webConfig) throws InitException{
    this.webConfig = webConfig;
    this.appBaseDir = appBaseDir;
    try{

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
      InformerSettings is = informerConfigManager.getConfigSettings();
      infosme = new InfosmeImpl(is.getHost(), is.getAdminPort());



      Properties pers = new Properties();
      pers.setProperty("personalization.host",is.getPersHost());
      pers.setProperty("personalization.port",Integer.toString(is.getPersPort()));

      personalizationClientPool = new PersonalizationClientPool(pers);

      blacklistManager = new BlackListManagerImpl(personalizationClientPool);
    }catch (AdminException e) {
      throw new InitException(e);
    }catch (PersonalizationClientException e) {
      throw new InitException(e);
    }
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  public void shutdown() {
    if(personalizationClientPool != null) {
      try{
        personalizationClientPool.shutdown();
      }catch (Exception e){}
    }
    if(infosme != null) {
      infosme.shutdown();
    }

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

  public void addInBlacklist(String msisdn) throws AdminException{
    blacklistManager.add(msisdn);
  }

  public void addInBlacklist(Collection<String> msisdn) throws AdminException{
    blacklistManager.add(msisdn);
  }

  public void removeFromBlacklist(String msisdn) throws AdminException{
    blacklistManager.remove(msisdn);
  }

  public void removeFromBlacklist(Collection<String> msisdns) throws AdminException{
    blacklistManager.remove(msisdns);
  }

  public boolean blacklistContains(String msisdn) throws AdminException{
    return blacklistManager.contains(msisdn);
  }

}

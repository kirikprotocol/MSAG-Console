package mobi.eyeline.informer.web.config;

import mobi.eyeline.informer.admin.AdminContext;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.web.informer.InformerConfigManager;
import mobi.eyeline.informer.web.informer.InformerSettings;
import mobi.eyeline.informer.web.journal.Journal;
import mobi.eyeline.informer.web.users.UsersManager;
import mobi.eyeline.informer.web.users.UsersSettings;

/**
 * @author Aleksandr Khalitov
 */
public class Configuration {

  protected final Journal journal;

  protected final AdminContext context;

  protected final UsersManager usersManager;

  protected UsersSettings usersSettings;
  
  protected InformerConfigManager informerConfigManager;
  
  protected InformerSettings informerSettings;

  public Configuration(Journal journal, AdminContext context, UsersManager usersManager, InformerConfigManager informerConfigManager) throws AdminException {
    this.journal = journal;
    this.context = context;
    this.usersManager = usersManager;
    this.usersSettings = usersManager.getUsersSettings();
    this.informerConfigManager = informerConfigManager;
    this.informerSettings = informerConfigManager.getConfigSettings();
  }

  public UsersSettings getUserSettings() throws AdminException {
    return usersSettings.cloneSettings();
  }

  public void setUserSettings(UsersSettings newS, String user) throws AdminException {
    journal.logChanges(usersSettings, newS, user);
    usersManager.updateSettings(newS);
    usersSettings = usersManager.getUsersSettings();
  }

  public InformerSettings getConfigSettings() throws AdminException {
    return informerSettings.cloneSettings();
  }

  public void setConfigSettings(InformerSettings newS, String user) throws AdminException{
    journal.logChanges(informerSettings, newS, user);
    informerConfigManager.updateSettings(newS);
    informerSettings = informerConfigManager.getConfigSettings();
  }
}

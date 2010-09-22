package mobi.eyeline.informer.web.config;

import mobi.eyeline.informer.admin.AdminContext;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.web.journal.Journal;
import mobi.eyeline.informer.web.users.UsersManager;
import mobi.eyeline.informer.web.users.UsersSettings;

/**
 * @author Aleksandr Khalitov
 */
public class Configuration {

  private final Journal journal;

  private final AdminContext context;

  private final UsersManager usersManager;

  protected UsersSettings usersSettings;

  public Configuration(Journal journal, AdminContext context, UsersManager usersManager) throws AdminException {
    this.journal = journal;
    this.context = context;
    this.usersManager = usersManager;
    this.usersSettings = usersManager.getUsersSettings();
  }

  public UsersSettings getUserSettings() throws AdminException {
    return usersSettings.cloneSettings();
  }

  public void setUserSettings(UsersSettings newS, String user) throws AdminException{
    journal.logChanges(usersSettings, newS, user);
    usersManager.updateSettings(newS);
    usersSettings = usersManager.getUsersSettings();
  }
}

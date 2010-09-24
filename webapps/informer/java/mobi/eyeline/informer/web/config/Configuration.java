package mobi.eyeline.informer.web.config;

import mobi.eyeline.informer.admin.AdminContext;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.config.Revision;
import mobi.eyeline.informer.admin.informer.InformerSettings;
import mobi.eyeline.informer.admin.journal.Journal;
import mobi.eyeline.informer.admin.users.UsersSettings;

import java.util.EnumMap;
import java.util.Map;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Класс для управления конфигурациями
 * @author Aleksandr Khalitov
 */
public class Configuration {

  private final Map<ConfigType, Revision> lastRevisions = new EnumMap<ConfigType, Revision>(ConfigType.class);

  private final Journal journal;

  private final AdminContext context;

  private UsersSettings usersSettings;

  private InformerSettings informerSettings;

  public Configuration(AdminContext context) throws AdminException {
    this.journal = context.getJournal();
    this.context = context;
    this.usersSettings = context.getUsersSettings();
    this.informerSettings = context.getConfigSettings();
  }

  public UsersSettings getUserSettings() throws AdminException {
    return usersSettings.cloneSettings();
  }

  public void setUserSettings(UsersSettings newS, String user) throws AdminException {
    journal.logChanges(usersSettings, newS, user);
    context.updateUserSettings(newS);
    usersSettings = context.getUsersSettings();
  }

  public InformerSettings getConfigSettings() throws AdminException {
    return informerSettings.cloneSettings();
  }

  public void setConfigSettings(InformerSettings newS, String user) throws AdminException{
    journal.logChanges(informerSettings, newS, user);
    context.updateConfigSettings(newS);
    informerSettings = context.getConfigSettings();
  }

  public Journal getJournal() {
    return journal;
  }

  private final Lock revisionLock = new ReentrantLock();

  public void lockRevision() {
    revisionLock.lock();
  }
  public void unlockRevision() {
    revisionLock.unlock();
  }

  public Revision getLastRevision(ConfigType configType) {
    return lastRevisions.get(configType);
  }

  public void setRevision(ConfigType configType, Revision r) {
    lastRevisions.put(configType, r);
  }

  public enum ConfigType {
    CONFIG, USERS
  }
}

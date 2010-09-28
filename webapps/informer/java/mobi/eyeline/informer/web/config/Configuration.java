package mobi.eyeline.informer.web.config;

import mobi.eyeline.informer.admin.AdminContext;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.informer.InformerSettings;
import mobi.eyeline.informer.admin.journal.Journal;
import mobi.eyeline.informer.admin.users.UsersSettings;

import java.util.Collection;
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

  public Configuration(AdminContext context) throws InitException {
    this.journal = context.getJournal();
    this.context = context;
    try{
      this.usersSettings = context.getUsersSettings();
      this.informerSettings = context.getConfigSettings();
    }catch (AdminException e){
      throw new InitException(e);
    }
  }

  public UsersSettings getUserSettings() throws AdminException {
    return usersSettings.cloneSettings();
  }

  public void setUserSettings(UsersSettings newS, String user) throws AdminException {
    Revision revision = buildNextRevision(user, ConfigType.USERS);
    context.updateUserSettings(newS);
    lastRevisions.put(ConfigType.USERS, revision);
    journal.logChanges(usersSettings, newS, user);
    usersSettings = context.getUsersSettings();
  }

  public InformerSettings getConfigSettings() throws AdminException {
    return informerSettings.cloneSettings();
  }

  private Revision buildNextRevision(String user, ConfigType type) {
    Revision revision = getLastRevision(type);
    if(revision == null) {
      revision = new Revision(user, 1);
    }else {
      revision = new Revision(user, revision.getNumber() + 1);
    }
    return revision;
  }

  public void setConfigSettings(InformerSettings newS, String user) throws AdminException{
    Revision revision = buildNextRevision(user, ConfigType.CONFIG);
    context.updateConfigSettings(newS);
    lastRevisions.put(ConfigType.CONFIG, revision);
    journal.logChanges(informerSettings, newS, user);
    informerSettings = context.getConfigSettings();
  }

  public Journal getJournal() {
    return journal;
  }

  public void addInBlacklist(Collection<String> msisdn) throws AdminException {
    context.addInBlacklist(msisdn);
  }

  public void addInBlacklist(String msisdn) throws AdminException {
    context.addInBlacklist(msisdn);
  }

  public void removeFromBlacklist(String msisdn) throws AdminException {
    context.removeFromBlacklist(msisdn);
  }

  public boolean blacklistContains(String msisdn) throws AdminException {
    return context.blacklistContains(msisdn);
  }

  private final Lock lock = new ReentrantLock();

  public void lock() {
    lock.lock();
  }
  public void unlock() {
    lock.unlock();
  }

  public Revision getLastRevision(ConfigType configType) {
    return lastRevisions.get(configType);
  }

  public enum ConfigType {
    CONFIG, USERS
  }
}

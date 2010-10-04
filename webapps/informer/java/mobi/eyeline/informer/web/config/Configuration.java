package mobi.eyeline.informer.web.config;

import mobi.eyeline.informer.admin.AdminContext;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.informer.InformerSettings;
import mobi.eyeline.informer.admin.journal.Journal;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.admin.users.UsersSettings;

import java.util.Collection;
import java.util.EnumMap;
import java.util.List;
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

  public void addInBlacklist(Collection<String> msisdns, String user) throws AdminException {
    context.addInBlacklist(msisdns);
    for(String msisdn: msisdns) {
      journal.logAddBlacklist(msisdn, user);
    }
  }

  public void addInBlacklist(String msisdn, String user) throws AdminException {
    context.addInBlacklist(msisdn);
    journal.logAddBlacklist(msisdn, user);
  }

  public void removeFromBlacklist(String msisdn, String user) throws AdminException {
    context.removeFromBlacklist(msisdn);
  }

  public void removeFromBlacklist(Collection<String> msisdns, String user) throws AdminException {
    context.removeFromBlacklist(msisdns);
    for(String msisdn : msisdns) {
      journal.logAddBlacklist(msisdn, user);
    }
  }

  public boolean blacklistContains(String msisdn) throws AdminException {
    return context.blacklistContains(msisdn);
  }

  public String getDefaultSmsc() {
    return context.getDefaultSmsc();
  }

  public void setDefaultSmsc(String smsc) throws AdminException {
    context.setDefaultSmsc(smsc);        //todo journal
  }

  public void removeSmsc(String smscName) throws AdminException {
    context.removeSmsc(smscName);        //todo journal
  }

  public Smsc getSmsc(String name) {
    return context.getSmsc(name);
  }

  public List<Smsc> getSmscs() {
    return context.getSmscs();
  }

  public void updateSmsc(Smsc smsc) throws AdminException {
    context.updateSmsc(smsc);            //todo journal
  }

  public void addSmsc(Smsc smsc) throws AdminException {
    context.addSmsc(smsc);               //todo journal
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

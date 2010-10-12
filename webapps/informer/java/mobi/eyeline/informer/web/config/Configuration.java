package mobi.eyeline.informer.web.config;

import mobi.eyeline.informer.admin.AdminContext;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.informer.InformerSettings;
import mobi.eyeline.informer.admin.journal.Journal;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.retry_policies.RetryPolicy;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.admin.users.UsersSettings;
import mobi.eyeline.informer.util.Address;

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
    journal.logRemoveBlacklist(msisdn, user);
  }

  public void removeFromBlacklist(Collection<String> msisdns, String user) throws AdminException {
    context.removeFromBlacklist(msisdns);
    for(String msisdn : msisdns) {
      journal.logRemoveBlacklist(msisdn, user);
    }
  }

  public boolean blacklistContains(String msisdn) throws AdminException {
    return context.blacklistContains(msisdn);
  }

  public String getDefaultSmsc() {
    return context.getDefaultSmsc();
  }

  public void setDefaultSmsc(String newSmsc, String user) throws AdminException {
    String oldSmsc = context.getDefaultSmsc();
    if(!oldSmsc.equals(newSmsc)) {
      context.setDefaultSmsc(newSmsc);
      journal.logSetDefaultSmsc(oldSmsc, newSmsc, user);
    }
  }

  public void removeSmsc(String smscName, String user) throws AdminException {
    context.removeSmsc(smscName);
    journal.logRemoveSmsc(smscName, user);
  }

  public Smsc getSmsc(String name) {
    return context.getSmsc(name);
  }

  public List<Smsc> getSmscs() {
    return context.getSmscs();
  }

  public void updateSmsc(Smsc smsc, String user) throws AdminException {
    Smsc oldSmsc = context.getSmsc(smsc.getName());
    context.updateSmsc(smsc);
    journal.logUpdateSmsc(oldSmsc, smsc, user);
  }

  public void addSmsc(Smsc smsc, String user) throws AdminException {
    context.addSmsc(smsc);
    journal.logAddSmsc(smsc.getName(), user);
  }

  public void removeRegion(String regionId, String user) throws AdminException {
    Region r = getRegion(regionId);
    if(r != null) {
      context.removeRegion(r.getRegionId());
      journal.logRemoveRegion(r.getName(), user);
    }
  }

  public Region getRegion(String id) {
    return context.getRegion(id);
  }

  public Region getRegion(Address a) {
    return context.getRegion(a);
  }


  public List<Region> getRegions() {
    return context.getRegions();
  }

  public void updateRegion(Region region, String user) throws AdminException {
    Region oldRegion = context.getRegion(region.getRegionId());
    context.updateRegion(region);
    journal.logUpdateRegion(oldRegion, region, user);
  }

  public void addRegion(Region region, String user) throws AdminException {
    context.addRegion(region);
    journal.logAddRegion(region.getName(), user);
  }
  
  public List<RetryPolicy> getRetryPolicies() throws AdminException {
    return context.getRetryPolicies();
  }

  public RetryPolicy getRetryPolicy(String policyId) throws AdminException {
    return context.getRetryPolicy(policyId);
  }

  public void updateRetryPolicy(RetryPolicy rp, String user) throws AdminException {
    RetryPolicy rpOld = context.getRetryPolicy(rp.getPolicyId());
    context.updateRetryPolicy(rp);
    journal.logUpdateRetryPolicy(rpOld,rp,user);
  }

  public void addRetryPolicy(RetryPolicy rp, String user) throws AdminException {
    context.addRetryPolicy(rp);
    journal.logAddRetryPolicy(rp,user);
  }

  public void removeRetryPolicy(String policyId, String user) throws AdminException {
    context.removeRetryPolicy(policyId);
    journal.logRemoveRetryPolicy(policyId,user);
  }

  public void startInformer(String user) throws AdminException {
    context.startInformer();
    journal.logInformerStart(user);
  }

  public void stopInformer(String user) throws AdminException {
    context.stopInformer();
    journal.logInformerStop(user);
  }

  public void switchInformer(String toHost, String user) throws AdminException {
    context.switchInformer(toHost);
    journal.logInformerSwitch(toHost, user);
  }

  public String getInformerOnlineHost() throws AdminException {
    return context.getInformerOnlineHost();
  }

  public List<String> getInformerHosts() throws AdminException {
    return context.getInformerHosts();
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
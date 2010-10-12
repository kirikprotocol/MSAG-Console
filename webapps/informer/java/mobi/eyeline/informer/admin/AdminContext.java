package mobi.eyeline.informer.admin;

import com.eyelinecom.whoisd.personalization.PersonalizationClientPool;
import com.eyelinecom.whoisd.personalization.exceptions.PersonalizationClientException;
import mobi.eyeline.informer.admin.blacklist.BlackListManagerImpl;
import mobi.eyeline.informer.admin.blacklist.BlacklistManager;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.informer.InformerManager;
import mobi.eyeline.informer.admin.informer.InformerManagerImpl;
import mobi.eyeline.informer.admin.informer.InformerSettings;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.infosme.protogen.InfosmeImpl;
import mobi.eyeline.informer.admin.journal.Journal;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.regions.RegionException;
import mobi.eyeline.informer.admin.regions.RegionsManager;
import mobi.eyeline.informer.admin.retry_policies.RetryPolicy;
import mobi.eyeline.informer.admin.retry_policies.RetryPolicyManager;
import mobi.eyeline.informer.admin.service.ServiceManager;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.admin.smsc.SmscException;
import mobi.eyeline.informer.admin.smsc.SmscManager;
import mobi.eyeline.informer.admin.users.UsersManager;
import mobi.eyeline.informer.admin.users.UsersSettings;
import mobi.eyeline.informer.util.Address;

import java.io.File;
import java.util.Collection;
import java.util.List;
import java.util.Properties;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

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

  protected InformerManager informerManager;

  protected Infosme infosme;

  protected BlacklistManager blacklistManager;

  protected PersonalizationClientPool personalizationClientPool;

  protected SmscManager smscManager;

  protected RegionsManager regionsManager;

  protected ServiceManager serviceManager;
  
  protected RetryPolicyManager retryPolicyManager;
  
  protected AdminContext() {
  }

  public AdminContext(File appBaseDir, WebConfig webConfig) throws InitException{
    this.webConfig = webConfig;
    this.appBaseDir = appBaseDir;
    try{

      this.instType = webConfig.getInstallationType();

      File servicesDir = new File(appBaseDir, "services");

      switch (this.instType) {
        case SINGLE:
          serviceManager = ServiceManager.getServiceManagerForSingleInst(webConfig.getSingleDaemonHost(), webConfig.getSingleDaemonPort(), servicesDir);
          fileSystem = FileSystem.getFSForSingleInst();
          break;
        case HS:
          serviceManager = ServiceManager.getServiceManagerForHSInst(webConfig.getHSDaemonHost(), webConfig.getHSDaemonPort(), servicesDir, webConfig.getHSDaemonHosts());
          fileSystem = FileSystem.getFSForHSInst(appBaseDir, webConfig.getAppMirrorDirs());
          break;
        default:
          serviceManager = ServiceManager.getServiceManagerForHAInst(new File(appBaseDir, "conf/resourceGroups.properties"), servicesDir);
          fileSystem = FileSystem.getFSForHAInst();
      }

      File usersFile = new File(webConfig.getUsersFile());
      journal = new Journal(new File(webConfig.getJournalDir()), fileSystem);
      informerManager = new InformerManagerImpl(new File(appBaseDir,"conf"+File.separatorChar+"config.xml"),
          new File(appBaseDir,"conf"+File.separatorChar+"backup"), fileSystem, serviceManager);
      usersManager = new UsersManager(usersFile, new File(usersFile.getParentFile(), "backup"), fileSystem);
      InformerSettings is = informerManager.getConfigSettings();
      infosme = new InfosmeImpl(is.getHost(), is.getAdminPort());

      Properties pers = new Properties();
      pers.setProperty("personalization.host",is.getPersHost());
      pers.setProperty("personalization.port",Integer.toString(is.getPersPort()));

      personalizationClientPool = new PersonalizationClientPool(pers);

      blacklistManager = new BlackListManagerImpl(personalizationClientPool);

      smscManager = new SmscManager(infosme, new File(appBaseDir,"conf"+File.separatorChar+"smsc.xml"),
          new File(appBaseDir,"conf"+File.separatorChar+"backup"), fileSystem);
      regionsManager = new RegionsManager(infosme, new File(appBaseDir,"conf"+File.separatorChar+"regions.xml"),
          new File(appBaseDir,"conf"+File.separatorChar+"backup"), fileSystem);

      retryPolicyManager = new RetryPolicyManager(infosme, new File(appBaseDir,"conf"+File.separatorChar+"policies.xml"),
          new File(appBaseDir,"conf"+File.separatorChar+"backup"), fileSystem);
      
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
    return informerManager.getConfigSettings();
  }

  public void updateConfigSettings(InformerSettings informerSettings) throws AdminException {
    informerManager.updateSettings(informerSettings);
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

  final private Lock regionsSmscLock = new ReentrantLock();

  public void addSmsc(Smsc smsc) throws AdminException {
    smscManager.addSmsc(smsc);
  }

  public void updateSmsc(Smsc smsc) throws AdminException {
    smscManager.updateSmsc(smsc);
  }

  public List<Smsc> getSmscs() {
    return smscManager.getSmscs();
  }

  public Smsc getSmsc(String name) {
    return smscManager.getSmsc(name);
  }

  public void removeSmsc(String smscName) throws AdminException {
    try{
      regionsSmscLock.lock();
      if(!regionsManager.getRegionsBySmsc(smscName).isEmpty()) {
        throw new SmscException("smsc_used_in_regions", smscName);
      }
      smscManager.removeSmsc(smscName);
    }finally {
      regionsSmscLock.unlock();
    }
  }

  public void setDefaultSmsc(String smsc) throws AdminException {
    smscManager.setDefaultSmsc(smsc);
  }

  public String getDefaultSmsc() {
    return smscManager.getDefaultSmsc();
  }

  public void addRegion(Region region) throws AdminException{
    try{
      regionsSmscLock.lock();
      if(smscManager.getSmsc(region.getSmsc()) == null) {
        throw new RegionException("smsc_not_exist", region.getSmsc());
      }
      regionsManager.addRegion(region);
    }finally {
      regionsSmscLock.unlock();
    }
  }

  public void updateRegion(Region region) throws AdminException{
    try{
      regionsSmscLock.lock();
      if(smscManager.getSmsc(region.getSmsc()) == null) {
        throw new RegionException("smsc_not_exist", region.getSmsc());
      }
      regionsManager.updateRegion(region);
    }finally {
      regionsSmscLock.unlock();
    }
  }

  public void removeRegion(String regionId) throws AdminException{
    regionsManager.removeRegion(regionId);
  }

  public Region getRegion(String regionId){
    return regionsManager.getRegion(regionId);
  }

  public Region getRegion(Address address){
    return regionsManager.getRegion(address);
  }

  public List<Region> getRegions() {
    return regionsManager.getRegions();
  }


  public RetryPolicy getRetryPolicy(String policyId) throws AdminException{
    return retryPolicyManager.getRetryPolicy(policyId);
  }

  public List<RetryPolicy> getRetryPolicies() throws AdminException{
    return retryPolicyManager.getRetryPolicies();
  }

  public void addRetryPolicy(RetryPolicy rp) throws AdminException{
    retryPolicyManager.addRetryPolicy(rp);
  }

  public void updateRetryPolicy(RetryPolicy rp) throws AdminException{
    retryPolicyManager.updateRetryPolicy(rp);
  }

  public void removeRetryPolicy(String policyId) throws AdminException{
    retryPolicyManager.removeRetryPolicy(policyId);
  }

  public void startInformer() throws AdminException {
    informerManager.startInformer();
  }

  public void stopInformer() throws AdminException {
    informerManager.stopInformer();
  }

  public void switchInformer(String toHost) throws AdminException {
    informerManager.switchInformer(toHost);
  }

  public String getInformerOnlineHost() throws AdminException {
    return informerManager.getInformerOnlineHost();
  }

  public List<String> getInformerHosts() throws AdminException {
    return informerManager.getInformerHosts();
  }
}
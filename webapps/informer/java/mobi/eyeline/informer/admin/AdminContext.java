package mobi.eyeline.informer.admin;

import com.eyelinecom.whoisd.personalization.PersonalizationClientPool;
import com.eyelinecom.whoisd.personalization.exceptions.PersonalizationClientException;
import mobi.eyeline.informer.admin.blacklist.BlackListManagerImpl;
import mobi.eyeline.informer.admin.blacklist.BlacklistManager;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.informer.InformerManager;
import mobi.eyeline.informer.admin.informer.InformerManagerImpl;
import mobi.eyeline.informer.admin.informer.InformerSettings;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.infosme.TestSms;
import mobi.eyeline.informer.admin.infosme.protogen.InfosmeImpl;
import mobi.eyeline.informer.admin.journal.Journal;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.regions.RegionsManager;
import mobi.eyeline.informer.admin.restriction.Restriction;
import mobi.eyeline.informer.admin.restriction.RestrictionDaemon;
import mobi.eyeline.informer.admin.restriction.RestrictionsFilter;
import mobi.eyeline.informer.admin.restriction.RestrictionsManager;
import mobi.eyeline.informer.admin.service.ServiceManager;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.admin.smsc.SmscException;
import mobi.eyeline.informer.admin.smsc.SmscManager;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UsersManager;
import mobi.eyeline.informer.util.Address;

import java.io.File;
import java.util.Collection;
import java.util.LinkedList;
import java.util.List;
import java.util.Properties;
import java.util.concurrent.ConcurrentHashMap;
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

  protected WebConfigManager webConfig;

  protected InformerManager informerManager;

  protected Infosme infosme;

  protected BlacklistManager blacklistManager;

  protected PersonalizationClientPool personalizationClientPool;

  protected SmscManager smscManager;

  protected RegionsManager regionsManager;

  protected ServiceManager serviceManager;

  protected DeliveryManager deliveryManager;

  protected File workDir;

  protected RestrictionsManager restrictionsManager;

  protected RestrictionDaemon restrictionDaemon;

// delivery ->user ->region->smsc

  final private Lock integrityLock = new ReentrantLock();

  final private ConcurrentHashMap<Integer, Object> deliveriesLock = new ConcurrentHashMap<Integer, Object>(10);

  protected AdminContext() {
  }

  public AdminContext(File appBaseDir, WebConfigManager webConfig) throws InitException{
    this.webConfig = webConfig;
    this.appBaseDir = appBaseDir;
    try{

      this.instType = webConfig.getInstallationType();

      File servicesDir = new File(appBaseDir, "services");

      File confDir = new File(servicesDir, "Informer"+File.separatorChar+"conf");

      workDir = new File(appBaseDir, "work");
      if(!workDir.exists() && !workDir.mkdirs()) {
        throw new InitException("Can't create work dir: "+workDir.getAbsolutePath());
      }

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


      journal = new Journal(new File(webConfig.getJournalDir()), fileSystem);
      informerManager = new InformerManagerImpl(new File(confDir,"config.xml"),
          new File(confDir, "backup"), fileSystem, serviceManager);

      InformerSettings is = informerManager.getConfigSettings();
      infosme = new InfosmeImpl(is.getAdminHost(), is.getAdminPort());

      usersManager = new UsersManager(infosme, new File(confDir, "users.xml"),new File(confDir, "backup"), fileSystem);

      Properties pers = new Properties();
      pers.setProperty("personalization.host",is.getPersHost());
      pers.setProperty("personalization.port",Integer.toString(is.getPersPort()));

      personalizationClientPool = new PersonalizationClientPool(pers);

      blacklistManager = new BlackListManagerImpl(personalizationClientPool);

      smscManager = new SmscManager(infosme, new File(confDir, "smsc.xml"),
          new File(confDir, "backup"), fileSystem);
      regionsManager = new RegionsManager(infosme, new File(confDir, "regions.xml"),
          new File(confDir, "backup"), fileSystem);

      deliveryManager = new DeliveryManager(is.getDcpHost(), is.getDcpPort(), new File(is.getStatDir()), fileSystem);

      restrictionsManager = new RestrictionsManager(infosme, new File(confDir, "restrictions.csv"),
          new File(confDir, "backup"), fileSystem);

      restrictionDaemon = new RestrictionDaemon(deliveryManager,restrictionsManager,usersManager);

    }catch (AdminException e) {
      throw new InitException(e);
    }catch (PersonalizationClientException e) {
      throw new InitException(e);
    }
  }

  public File getWorkDir() {
    return new File(workDir.getAbsolutePath());
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  public void shutdown() {
    if(personalizationClientPool != null) {
      try{
        personalizationClientPool.shutdown();
      }catch (Exception e){}
    }
    if(infosme != null) {
      try{
        infosme.shutdown();
      }catch (Exception e){}
    }
    if(deliveryManager != null) {
      try{
        deliveryManager.shutdown();
      }catch (Exception e){}

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

  public User getUser(String login)  {
    return usersManager.getUser(login);
  }

  public List<User> getUsers()  {
    return usersManager.getUsers();
  }

  public void updateUser(User u) throws AdminException {
    try{
      integrityLock.lock();
      if(u.getRegions()!=null) {
        for(Integer rId : u.getRegions()) {
          if(null == regionsManager.getRegion(rId)) {
            throw new IntegrityException("user.region.not.exists",u.getLogin(),rId.toString());
          }
        }
      }
      User oldUser = usersManager.getUser(u.getLogin());
      usersManager.updateUser(u);
      if(oldUser.getStatus()!=u.getStatus()) {
        restrictionDaemon.rebuildSchedule();
      }
    }
    finally {
      integrityLock.unlock();
    }
  }

  public void addUser(User u) throws AdminException {
    try{
      integrityLock.lock();
      if(u.getRegions()!=null) {
        for(Integer rId : u.getRegions()) {
          if(null == regionsManager.getRegion(rId)) {
            throw new IntegrityException("user.region.not.exists",u.getLogin(),rId.toString());
          }
        }
      }
      usersManager.addUser(u);      
    }
    finally {
      integrityLock.unlock();
    }
  }

  public void removeUser(String login) throws AdminException {
    try{
      integrityLock.lock();
      User user = usersManager.getUser(login);
      DeliveryFilter filter = new DeliveryFilter();
      filter.setUserIdFilter(new String[]{login});
      final boolean[] notExist = new boolean[]{true};
      deliveryManager.getDeliveries(user.getLogin(), user.getPassword(), filter , 1, new Visitor<mobi.eyeline.informer.admin.delivery.DeliveryInfo>() {
        public boolean visit(DeliveryInfo value) throws AdminException {
          notExist[0] = false;
          return false;
        }
      });
      if(!notExist[0]) {
        throw new IntegrityException("fail.delete.user.by.delivery", login);
      }
      usersManager.removeUser(login);
    }
    finally {
      integrityLock.unlock();
    }
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
      integrityLock.lock();
      if(!regionsManager.getRegionsBySmsc(smscName).isEmpty()) {
        throw new SmscException("smsc_used_in_regions", smscName);
      }
      smscManager.removeSmsc(smscName);
    }finally {
      integrityLock.unlock();
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
      integrityLock.lock();
      if(smscManager.getSmsc(region.getSmsc()) == null) {
        throw new IntegrityException("smsc_not_exist", region.getSmsc());
      }
      regionsManager.addRegion(region);
    }finally {
      integrityLock.unlock();
    }
  }

  public void updateRegion(Region region) throws AdminException{
    try{
      integrityLock.lock();
      if(smscManager.getSmsc(region.getSmsc()) == null) {
        throw new IntegrityException("smsc_not_exist", region.getSmsc());
      }
      regionsManager.updateRegion(region);
    }finally {
      integrityLock.unlock();
    }
  }

  public int getDefaultMaxPerSecond() {
    return regionsManager.getDefaultMaxPerSecond();
  }

  public void setDefaultMaxPerSecond(int defMaxPerSecond) throws AdminException {
    regionsManager.setDefaultMaxPerSecond(defMaxPerSecond);
  }

  public void removeRegion(Integer regionId) throws AdminException{
    try{
      integrityLock.lock();
      for(User u : usersManager.getUsers()) {
        for(Integer s : u.getRegions()) {
          if(s.equals(regionId)) {
            throw new IntegrityException("fail.delete.region.to.user",regionsManager.getRegion(regionId).getName(),u.getLogin());
          }
        }
      }
      regionsManager.removeRegion(regionId);
    }
    finally {
      integrityLock.unlock();
    }
  }

  public Region getRegion(Integer regionId){
    return regionsManager.getRegion(regionId);
  }

  public Region getRegion(Address address){
    return regionsManager.getRegion(address);
  }

  public List<Region> getRegions() {
    return regionsManager.getRegions();
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

  public void statistics(DeliveryStatFilter filter, DeliveryStatVisitor visitor) throws AdminException {
    deliveryManager.statistics(filter, visitor);
  }

  public List<Daemon> getDaemons() {
    List<Daemon> ret = new LinkedList<Daemon>();
    ret.add(restrictionDaemon);
    return ret;
  }

  private Object getLock(int deliveryId) {
    Object lock = deliveriesLock.get(deliveryId);
    if(lock == null) {
      Object l = deliveriesLock.putIfAbsent(deliveryId, lock = new Object());
      if(l != null) {
        lock = l;
      }
    }
    return lock;
  }

  public void createDelivery(String login, String password, Delivery delivery, DataSource<Message> msDataSource) throws AdminException {
    if(restrictionsManager.hasActiveRestriction(login)) {
      throw new DeliveryException("creation_restricted");
    }
    try{
      integrityLock.lock();
      if(usersManager.getUser(delivery.getOwner()) == null) {
        throw new IntegrityException("user_not_exist", delivery.getOwner());
      }
      deliveryManager.createDelivery(login, password, delivery, msDataSource);
    }finally {
      integrityLock.unlock();
    }
  }

  public void createSingleTextDelivery(String login, String password, Delivery delivery, DataSource<Address> msDataSource) throws AdminException {
    try{
      integrityLock.lock();
      if(usersManager.getUser(delivery.getOwner()) == null) {
        throw new IntegrityException("user_not_exist", delivery.getOwner());
      }
      deliveryManager.createSingleTextDelivery(login, password, delivery, msDataSource);
    }finally {
      integrityLock.unlock();
    }
  }

  public void modifyDelivery(String login, String password, Delivery delivery) throws AdminException {
    try{
      integrityLock.lock();
      if(usersManager.getUser(delivery.getOwner()) == null) {
        throw new IntegrityException("user_not_exist", delivery.getOwner());
      }
      synchronized (getLock(delivery.getId())) {
        deliveryManager.modifyDelivery(login, password, delivery);
      }
    }finally {
      integrityLock.unlock();
    }
  }

  public void dropDelivery(String login, String password, int deliveryId) throws AdminException {
    synchronized (getLock(deliveryId)) {
      deliveryManager.setDeliveryRestriction(login, password, deliveryId, false);
      deliveryManager.dropDelivery(login, password, deliveryId);
    }
  }

  public void addMessages(String login, String password, DataSource<Message> msDataSource, int deliveryId) throws AdminException {

    synchronized (getLock(deliveryId)) {
      deliveryManager.addMessages(login, password, msDataSource, deliveryId);
    }
  }

  public List<Long> addSingleTextMessages(String login, String password, DataSource<Address> msDataSource, int deliveryId) throws AdminException {
    synchronized (getLock(deliveryId)) {
      return deliveryManager.addSingleTextMessages(login, password, msDataSource, deliveryId);
    }
  }

  public void dropMessages(String login, String password, int deliveryId, Collection<Long> messageIds) throws AdminException {
    synchronized (getLock(deliveryId)) {
      deliveryManager.dropMessages(login, password, deliveryId, messageIds);
    }
  }

  public int countDeliveries(String login, String password, DeliveryFilter deliveryFilter) throws AdminException {
    return deliveryManager.countDeliveries(login, password, deliveryFilter);
  }

  public Delivery getDelivery(String login, String password, int deliveryId) throws AdminException {
    return deliveryManager.getDelivery(login, password, deliveryId);
  }

  public void cancelDelivery(String login, String password, int deliveryId) throws AdminException {
    synchronized (getLock(deliveryId)) {
      deliveryManager.setDeliveryRestriction(login, password, deliveryId, false);
      deliveryManager.cancelDelivery(login, password, deliveryId);
    }
  }

  public void pauseDelivery(String login, String password, int deliveryId) throws AdminException {
    synchronized (getLock(deliveryId)) {
      deliveryManager.setDeliveryRestriction(login, password, deliveryId, false);
      deliveryManager.pauseDelivery(login, password, deliveryId);
    }
  }

  public void activateDelivery(String login, String password, int deliveryId) throws AdminException {
    synchronized (getLock(deliveryId)) {
      if(restrictionsManager.hasActiveRestriction(login)) {
        throw new DeliveryException("activation_restricted");
      }
      deliveryManager.activateDelivery(login, password, deliveryId);
    }
  }

  public DeliveryStatistics getDeliveryStats(String login, String password, int deliveryId) throws AdminException {
    synchronized (getLock(deliveryId)) {
      return deliveryManager.getDeliveryStats(login, password, deliveryId);
    }
  }

  public void getDeliveries(String login, String password, DeliveryFilter deliveryFilter, int _pieceSize, Visitor<DeliveryInfo> visitor) throws AdminException {
    deliveryManager.getDeliveries(login, password, deliveryFilter, _pieceSize, visitor);
  }

  public void getMessagesStates(String login, String password, MessageFilter filter, int _pieceSize, Visitor<MessageInfo> visitor) throws AdminException {
    synchronized (getLock(filter.getDeliveryId())) {
      deliveryManager.getMessages(login, password, filter, _pieceSize, visitor);
    }
  }

  public int countMessages(String login, String password, MessageFilter messageFilter) throws AdminException {
    synchronized (getLock(messageFilter.getDeliveryId())) {
      return deliveryManager.countMessages(login, password, messageFilter);
    }
  }

  public DeliveryStatusHistory getDeliveryStatusHistory(String login, String password, int deliverId) throws AdminException {
    synchronized (getLock(deliverId)) {
      return deliveryManager.getDeliveryStatusHistory(login, password, deliverId);
    }
  }

  public Restriction getRestriction(int id) {
    return restrictionsManager.getRestriction(id);
  }

  public List<Restriction> getRestrictions(RestrictionsFilter filter) {
    return restrictionsManager.getRestrictions(filter);
  }

  public void addRestriction(Restriction r) throws AdminException {
    restrictionsManager.addRestriction(r);
    restrictionDaemon.rebuildSchedule();
  }

  public void updateRestriction(Restriction r) throws AdminException {
    restrictionsManager.updateRestriction(r);
    restrictionDaemon.rebuildSchedule();
  }

  public void deleteRestriction(int id) throws AdminException {
    restrictionsManager.deleteRestriction(id);
    restrictionDaemon.rebuildSchedule();
  }

  public Delivery setDeliveryRestriction(String login, String password, int deliveryId, boolean restriction) throws AdminException {
    synchronized (getLock(deliveryId)) {
      return deliveryManager.setDeliveryRestriction(login, password, deliveryId, restriction);
    }
  }

  public void sendTestSms(TestSms sms) throws AdminException {
    infosme.sendTestSms(sms);
  }

  public Properties getJavaMailProperties() {
    return webConfig.getJavaMailProperties();
  }

  public void setJavaMailProperties(Properties props) throws AdminException {
    webConfig.setJavaMailProperties(props);
  }

  public Properties getNotificationTemplates() {
    return webConfig.getNotificationTemplates();
  }

  public void setNotificationTemplates(Properties props) throws AdminException {
    webConfig.setNotificationTemplates(props);
  }

  public Address getSmsSenderAddress() {
    return webConfig.getSmsSenderAddress();
  }

  public void setSmsSenderAddress(Address addr) throws AdminException {
    webConfig.setSmsSenderAddress(addr);
  }
}
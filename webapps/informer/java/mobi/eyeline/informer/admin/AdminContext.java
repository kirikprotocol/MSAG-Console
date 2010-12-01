package mobi.eyeline.informer.admin;

import com.eyelinecom.whoisd.personalization.PersonalizationClientPool;
import com.eyelinecom.whoisd.personalization.exceptions.PersonalizationClientException;
import mobi.eyeline.informer.admin.blacklist.BlackListManagerImpl;
import mobi.eyeline.informer.admin.blacklist.BlacklistManager;
import mobi.eyeline.informer.admin.cdr.CdrDaemon;
import mobi.eyeline.informer.admin.cdr.CdrDeliveries;
import mobi.eyeline.informer.admin.cdr.CdrUsers;
import mobi.eyeline.informer.admin.contentprovider.ContentProviderContext;
import mobi.eyeline.informer.admin.contentprovider.ContentProviderDaemon;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangesDetector;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatFilter;
import mobi.eyeline.informer.admin.delivery.stat.DeliveryStatVisitor;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.informer.InformerManager;
import mobi.eyeline.informer.admin.informer.InformerManagerImpl;
import mobi.eyeline.informer.admin.informer.InformerSettings;
import mobi.eyeline.informer.admin.infosme.Infosme;
import mobi.eyeline.informer.admin.infosme.TestSms;
import mobi.eyeline.informer.admin.infosme.protogen.InfosmeImpl;
import mobi.eyeline.informer.admin.journal.Journal;
import mobi.eyeline.informer.admin.notifications.DateAndFile;
import mobi.eyeline.informer.admin.notifications.DeliveryNotificationsContext;
import mobi.eyeline.informer.admin.notifications.DeliveryNotificationsDaemon;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.regions.RegionsManager;
import mobi.eyeline.informer.admin.restriction.Restriction;
import mobi.eyeline.informer.admin.restriction.RestrictionDaemon;
import mobi.eyeline.informer.admin.restriction.RestrictionsFilter;
import mobi.eyeline.informer.admin.restriction.RestrictionsManager;
import mobi.eyeline.informer.admin.service.ServiceManager;
import mobi.eyeline.informer.admin.siebel.SiebelException;
import mobi.eyeline.informer.admin.siebel.SiebelManager;
import mobi.eyeline.informer.admin.siebel.impl.SiebelDeliveries;
import mobi.eyeline.informer.admin.siebel.impl.SiebelFinalStateListener;
import mobi.eyeline.informer.admin.siebel.impl.SiebelRegionManager;
import mobi.eyeline.informer.admin.siebel.impl.SiebelUserManager;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.admin.smsc.SmscException;
import mobi.eyeline.informer.admin.smsc.SmscManager;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserException;
import mobi.eyeline.informer.admin.users.UsersManager;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;
import org.apache.log4j.Logger;

import java.io.File;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Класс для управления моделью
 *
 * @author Aleksandr Khalitov
 */
public class AdminContext {

  protected static final Logger logger = Logger.getLogger(AdminContext.class);

  protected FileSystem fileSystem;

  private InstallationType instType;

  protected Journal journal;

  protected UsersManager usersManager;

  protected WebConfigManager webConfig;

  protected InformerManager informerManager;

  protected Infosme infosme;

  protected BlacklistManager blacklistManager;

  private PersonalizationClientPool personalizationClientPool;

  protected SmscManager smscManager;

  protected RegionsManager regionsManager;

  protected ServiceManager serviceManager;

  protected DeliveryManager deliveryManager;

  protected File workDir;

  protected RestrictionsManager restrictionsManager;

  protected RestrictionDaemon restrictionDaemon;

  protected DeliveryChangesDetector deliveryChangesDetector;

  protected SiebelManager siebelManager;

  protected SiebelFinalStateListener siebelFinalStateListener;

  protected DeliveryNotificationsDaemon deliveryNotificationsDaemon;

  protected ContentProviderDaemon contentProviderDaemon;

  protected CdrDaemon cdrDaemon;

// delivery ->user ->region->smsc

  final private Lock integrityLock = new ReentrantLock();

  final private ConcurrentHashMap<Integer, Object> deliveriesLock = new ConcurrentHashMap<Integer, Object>(10);

  protected AdminContext() {
  }

  public AdminContext(File appBaseDir) throws InitException {

    try {
      File webConfDir = new File(appBaseDir, "conf");
      this.webConfig = new WebConfigManager(new File(webConfDir, "webconfig.xml"), new File(webConfDir, "backup"), FileSystem.getFSForSingleInst());

      this.instType = webConfig.getInstallationType();

      File servicesDir = new File(appBaseDir, "services");

      File confDir = new File(servicesDir, "Informer" + File.separatorChar + "conf");

      workDir = new File(appBaseDir, "work");
      if (!workDir.exists() && !workDir.mkdirs()) {
        throw new InitException("Can't create work dir: " + workDir.getAbsolutePath());
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
      informerManager = new InformerManagerImpl(new File(confDir, "config.xml"),
          new File(confDir, "backup"), fileSystem, serviceManager);

      InformerSettings is = informerManager.getConfigSettings();
      infosme = new InfosmeImpl(is.getAdminHost(), is.getAdminPort());

      usersManager = new UsersManager(infosme, new File(confDir, "users.xml"), new File(confDir, "backup"), fileSystem);

      Properties pers = new Properties();
      pers.setProperty("personalization.host", is.getPersHost());
      pers.setProperty("personalization.port", Integer.toString(is.getPersSyncPort()));

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


      contentProviderDaemon = new ContentProviderDaemon(new ContentProviderContextImpl(this),appBaseDir,workDir);


      deliveryChangesDetector = new DeliveryChangesDetector(new File(is.getStoreDir(), "final_log"), fileSystem);
      deliveryNotificationsDaemon = new DeliveryNotificationsDaemon(new DeliveryNotificationsContextImpl(this));
      deliveryChangesDetector.addListener(deliveryNotificationsDaemon);

      restrictionDaemon.start();

      try{
        initSiebel(workDir);
      }catch (Exception e){
        logger.error(e,e);
      }

      contentProviderDaemon.start();


      cdrDaemon = new CdrDaemon(new File(workDir, "cdr"),
          new File(webConfig.getCdrProperties().getProperty(CdrDaemon.CDR_DIR)),
          fileSystem,
          new CdrDeliveriesImpl(this), new CdrUsersImpl(this));
      cdrDaemon.start();

      deliveryChangesDetector.addListener(cdrDaemon);


      deliveryChangesDetector.start();


    } catch (AdminException e) {
      throw new InitException(e);
    } catch (PersonalizationClientException e) {
      throw new InitException(e);
    }
  }

  protected void initSiebel(File workDir) throws AdminException, InitException {
    SiebelDeliveries siebelDeliveries = new SiebelDeliveriesImpl(this);
    SiebelRegionManager siebelRegions = new SiebelRegionManagerImpl(this);
    SiebelUserManager userManager = new SiebelUserManagerImpl(this);

    User siebelUser = usersManager.getUser(webConfig.getSiebelProperties().getProperty(SiebelManager.USER));

    if (siebelUser == null) {
      throw new IntegrityException("user_not_exist", webConfig.getSiebelProperties().getProperty(SiebelManager.USER));
    }

    siebelManager = new SiebelManager(siebelDeliveries, siebelRegions);

    siebelFinalStateListener = new SiebelFinalStateListener(siebelManager, siebelDeliveries,
        userManager, workDir,
        Integer.parseInt(webConfig.getSiebelProperties().getProperty(SiebelFinalStateListener.PERIOD_PARAM)));

    deliveryChangesDetector.addListener(siebelFinalStateListener);

    siebelManager.start(siebelUser, webConfig.getSiebelProperties());

    siebelFinalStateListener.start();

  }

  private void shutdownSiebel() {
    if (siebelFinalStateListener != null) {
      if (deliveryChangesDetector != null) {
        try {
          deliveryChangesDetector.removeListener(siebelFinalStateListener);
        } catch (Exception ignored) {
        }
      }

      try {
        siebelFinalStateListener.shutdown();
      } catch (Exception ignored) {
      }
    }

    if (siebelManager != null) {
      try {
        siebelManager.stop();
      } catch (Exception ignored) {
      }
    }
  }

  private void shutdownCdr() {
    if (cdrDaemon != null) {
      if (deliveryChangesDetector != null) {
        try {
          deliveryChangesDetector.removeListener(cdrDaemon);
        } catch (Exception ignored) {
        }
      }
      try {
        cdrDaemon.stop();
      } catch (Exception ignored) {
      }
    }
  }

  public boolean isCdrStarted() {
    return cdrDaemon != null && cdrDaemon.istStrated();
  }

  public File getWorkDir() {
    return new File(workDir.getAbsolutePath());
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  public void shutdown() {
    shutdownSiebel();
    shutdownCdr();
    if(contentProviderDaemon!= null) {
      try{
        contentProviderDaemon.stop();
      }catch(Exception e){}
    }
    if(restrictionDaemon != null) {
      try{
        restrictionDaemon.stop();
      } catch (Exception e) {
      }
    }
    if (deliveryChangesDetector != null) {
      try {
        deliveryChangesDetector.shutdown();
      } catch (Exception e) {
      }
    }
    if (deliveryNotificationsDaemon != null) {
      try {
        deliveryNotificationsDaemon.shutdown();
      } catch (Exception e) {
      }
    }

    if (personalizationClientPool != null) {
      try {
        personalizationClientPool.shutdown();
      } catch (Exception e) {
      }
    }
    if (infosme != null) {
      try {
        infosme.shutdown();
      } catch (Exception e) {
      }
    }
    if (deliveryManager != null) {
      try {
        deliveryManager.shutdown();
      } catch (Exception e) {
      }
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

  public User getUser(String login) {
    return usersManager.getUser(login);
  }

  public List<User> getUsers() {
    return usersManager.getUsers();
  }

  public void updateUser(User u) throws AdminException {
    try {
      integrityLock.lock();
      if (u.getRegions() != null) {
        for (Integer rId : u.getRegions()) {
          if (null == regionsManager.getRegion(rId)) {
            throw new IntegrityException("user.region.not.exists", u.getLogin(), rId.toString());
          }
        }
      }
      User oldUser = usersManager.getUser(u.getLogin());

      if(!u.isCreateCDR() && oldUser.isCreateCDR()) {
        DeliveryFilter filter = new DeliveryFilter();
        filter.setUserIdFilter(u.getLogin());
        final boolean[] exist = new boolean[]{false};
        filter.setStatusFilter(DeliveryStatus.Planned, DeliveryStatus.Active, DeliveryStatus.Cancelled, DeliveryStatus.Paused);
        deliveryManager.getDeliveries(u.getLogin(), u.getPassword(), filter, 1000, new Visitor<Delivery>() {
          public boolean visit(Delivery value) throws AdminException {
            if(value.isEnableMsgFinalizationLogging()) {
              exist[0] = true;
              return false;
            }
            return true;
          }
        });
        if(exist[0]) {
          throw new IntegrityException("user.cdr.cant.disable", u.getLogin());
        }
      }

      usersManager.updateUser(u);
      if (oldUser.getStatus() != u.getStatus()) {
        restrictionDaemon.rebuildSchedule();
      }
    }
    finally {
      integrityLock.unlock();
    }
  }

  public void addUser(User u) throws AdminException {
    try {
      integrityLock.lock();
      if (u.getRegions() != null) {
        for (Integer rId : u.getRegions()) {
          if (null == regionsManager.getRegion(rId)) {
            throw new IntegrityException("user.region.not.exists", u.getLogin(), rId.toString());
          }
        }
      }
      usersManager.addUser(u);
    }
    finally {
      integrityLock.unlock();
    }
  }

  public boolean checkSiebelProperties(Properties p) {
    if(siebelManager != null) {
      try{
        siebelManager.checkProperties(p);
        return true;
      }catch (AdminException ignored) {}
    }
    return false;
  }

  public void removeUser(String login) throws AdminException {
    try {
      integrityLock.lock();
      if (login.equals(webConfig.getSiebelProperties().getProperty(SiebelManager.USER))) {
        throw new IntegrityException("fail.delete.user.siebel", login);
      }
      User user = usersManager.getUser(login);
      DeliveryFilter filter = new DeliveryFilter();
      filter.setUserIdFilter(login);
      final String[] exist = new String[]{null};
      deliveryManager.getDeliveries(user.getLogin(), user.getPassword(), filter, 1, new Visitor<mobi.eyeline.informer.admin.delivery.Delivery>() {
        public boolean visit(Delivery value) throws AdminException {
          exist[0] = value.getName();
          return false;
        }
      });
      if (exist[0] != null) {
        throw new IntegrityException("fail.delete.user.by.delivery", login, exist[0]);
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

  public void addInBlacklist(String msisdn) throws AdminException {
    blacklistManager.add(msisdn);
  }

  public void addInBlacklist(Collection<String> msisdn) throws AdminException {
    blacklistManager.add(msisdn);
  }

  public void removeFromBlacklist(String msisdn) throws AdminException {
    blacklistManager.remove(msisdn);
  }

  public void removeFromBlacklist(Collection<String> msisdns) throws AdminException {
    blacklistManager.remove(msisdns);
  }

  public boolean blacklistContains(String msisdn) throws AdminException {
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
    try {
      integrityLock.lock();
      if (!regionsManager.getRegionsBySmsc(smscName).isEmpty()) {
        throw new SmscException("smsc_used_in_regions", smscName);
      }
      smscManager.removeSmsc(smscName);
    } finally {
      integrityLock.unlock();
    }
  }

  public void setDefaultSmsc(String smsc) throws AdminException {
    smscManager.setDefaultSmsc(smsc);
  }

  public String getDefaultSmsc() {
    return smscManager.getDefaultSmsc();
  }

  public void addRegion(Region region) throws AdminException {
    try {
      integrityLock.lock();
      if (smscManager.getSmsc(region.getSmsc()) == null) {
        throw new IntegrityException("smsc_not_exist", region.getSmsc());
      }
      regionsManager.addRegion(region);
    } finally {
      integrityLock.unlock();
    }
  }

  public void updateRegion(Region region) throws AdminException {
    try {
      integrityLock.lock();
      if (smscManager.getSmsc(region.getSmsc()) == null) {
        throw new IntegrityException("smsc_not_exist", region.getSmsc());
      }
      regionsManager.updateRegion(region);
    } finally {
      integrityLock.unlock();
    }
  }

  public int getDefaultMaxPerSecond() {
    return regionsManager.getDefaultMaxPerSecond();
  }

  public void setDefaultMaxPerSecond(int defMaxPerSecond) throws AdminException {
    regionsManager.setDefaultMaxPerSecond(defMaxPerSecond);
  }

  public void removeRegion(Integer regionId) throws AdminException {
    try {
      integrityLock.lock();
      for (User u : usersManager.getUsers()) {
        if (u.getRegions() == null)
          continue;
        List<Integer> regions = u.getRegions();
        for (Iterator<Integer> iter = regions.iterator(); iter.hasNext();) {
          if (iter.next().equals(regionId)) {
            iter.remove();
            u.setRegions(regions);
            usersManager.updateUser(u);
            break;
          }
        }
      }
      regionsManager.removeRegion(regionId);
    }
    finally {
      integrityLock.unlock();
    }
  }

  public Region getRegion(Integer regionId) {
    return regionsManager.getRegion(regionId);
  }

  public Region getRegion(Address address) {
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

  public List<File> getStatisticsFiles(DeliveryStatFilter filter, boolean endDateInclusive) throws AdminException {
    return deliveryManager.getStatisticsFiles(filter, endDateInclusive);
  }

  public Calendar getCalendarOfStatFile(File f) throws AdminException {
    return deliveryManager.getCalendarOfStatFile(f);
  }

  public List<DateAndFile> getProcessedNotificationsFiles(Date startDate,Date endDate) throws AdminException {
    return deliveryChangesDetector.getProcessedNotificationsFiles(startDate,endDate);
  }


  public List<Daemon> getDaemons() {
    List<Daemon> ret = new LinkedList<Daemon>();
    ret.add(restrictionDaemon);
    ret.add(contentProviderDaemon);
    return ret;
  }

  public boolean isSiebelDaemonStarted() {
    return siebelManager.isStarted();
  }

  private Object getLock(int deliveryId) {
    Object lock = deliveriesLock.get(deliveryId);
    if (lock == null) {
      Object l = deliveriesLock.putIfAbsent(deliveryId, lock = new Object());
      if (l != null) {
        lock = l;
      }
    }
    return lock;
  }

  public Delivery createDeliveryWithIndividualTexts(String login, String password, DeliveryPrototype delivery, DataSource<Message> msDataSource) throws AdminException {
    if (restrictionsManager.hasActiveRestriction(login)) {
      throw new DeliveryException("creation_restricted");
    }
    try {
      integrityLock.lock();
      User u = usersManager.getUser(delivery.getOwner());
      if (u == null) {
        throw new IntegrityException("user_not_exist", delivery.getOwner());
      }
      if(u.isCreateCDR()) {
        delivery.setEnableMsgFinalizationLogging(true);
      }
      return deliveryManager.createDeliveryWithIndividualTexts(login, password, delivery, msDataSource);
    } finally {
      integrityLock.unlock();
    }
  }

  public Delivery createDeliveryWithSingleText(String login, String password, DeliveryPrototype delivery, DataSource<Address> msDataSource) throws AdminException {
    try {
      integrityLock.lock();
      User u = usersManager.getUser(delivery.getOwner());
      if (u == null) {
        throw new IntegrityException("user_not_exist", delivery.getOwner());
      }
      if(u.isCreateCDR()) {
        delivery.setEnableMsgFinalizationLogging(true);
      }
      return deliveryManager.createDeliveryWithSingleText(login, password, delivery, msDataSource);
    } finally {
      integrityLock.unlock();
    }
  }

  public void modifyDelivery(String login, String password, Delivery delivery) throws AdminException {
    try {
      integrityLock.lock();
      if (usersManager.getUser(delivery.getOwner()) == null) {
        throw new IntegrityException("user_not_exist", delivery.getOwner());
      }
      synchronized (getLock(delivery.getId())) {
        deliveryManager.modifyDelivery(login, password, delivery);
      }
    } finally {
      integrityLock.unlock();
    }
  }

  public Delivery setDeliveryRestriction(String login, String password, int deliveryId, boolean restriction) throws AdminException {
    synchronized (getLock(deliveryId)) {
      Delivery d = deliveryManager.getDelivery(login, password, deliveryId);
      d.setProperty(UserDataConsts.RESTRICTION, Boolean.toString(restriction));
      deliveryManager.modifyDelivery(login, password, d);
      return d;
    }
  }

  public void dropDelivery(String login, String password, int deliveryId) throws AdminException {
    synchronized (getLock(deliveryId)) {
      setDeliveryRestriction(login, password, deliveryId, false);
      deliveryManager.dropDelivery(login, password, deliveryId);
    }
  }

  public void addMessages(String login, String password, DataSource<Message> msDataSource, int deliveryId) throws AdminException {

    synchronized (getLock(deliveryId)) {
      deliveryManager.addIndividualMessages(login, password, msDataSource, deliveryId);
    }
  }

  public void addSingleTextMessages(String login, String password, DataSource<Address> msDataSource, int deliveryId) throws AdminException {
    synchronized (getLock(deliveryId)) {
      deliveryManager.addSingleTextMessages(login, password, msDataSource, deliveryId);
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
      setDeliveryRestriction(login, password, deliveryId, false);
      deliveryManager.cancelDelivery(login, password, deliveryId);
    }
  }

  public void pauseDelivery(String login, String password, int deliveryId) throws AdminException {
    synchronized (getLock(deliveryId)) {
      setDeliveryRestriction(login, password, deliveryId, false);
      deliveryManager.pauseDelivery(login, password, deliveryId);
    }
  }

  public void activateDelivery(String login, String password, int deliveryId) throws AdminException {
    synchronized (getLock(deliveryId)) {
      if (restrictionsManager.hasActiveRestriction(login)) {
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

  public void getDeliveries(String login, String password, DeliveryFilter deliveryFilter, int _pieceSize, Visitor<Delivery> visitor) throws AdminException {
    deliveryManager.getDeliveries(login, password, deliveryFilter, _pieceSize, visitor);
  }

  public void getMessagesStates(String login, String password, MessageFilter filter, int _pieceSize, Visitor<Message> visitor) throws AdminException {
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

  public static void getDefaultDelivery(User u, DeliveryPrototype delivery) throws AdminException {
    delivery.setOwner(u.getLogin());

    if (u.getSourceAddr() != null) {
      delivery.setSourceAddress(u.getSourceAddr());
    }
    if (u.getEmail() != null) {
      delivery.setProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS, u.getEmail());
    }
    if (u.getPhone() != null) {
      delivery.setProperty(UserDataConsts.SMS_NOTIF_ADDRESS, u.getPhone());
    }
    if (u.getDeliveryType() != null) {
      switch (u.getDeliveryType()) {
        case SMS:
          delivery.setDeliveryMode(DeliveryMode.SMS);
          break;
        case USSD_PUSH:
          delivery.setDeliveryMode(DeliveryMode.USSD_PUSH);
          delivery.setTransactionMode(true);
          break;
        case USSD_PUSH_VIA_VLR:
          delivery.setDeliveryMode(DeliveryMode.USSD_PUSH_VLR);
          delivery.setTransactionMode(true);
          break;
      }
    }
    if (u.isRetryOnFail()) {
      delivery.setRetryPolicy(u.getPolicyId());
      delivery.setRetryOnFail(true);
    }
    try {
      delivery.setPriority(u.getPriority());
    } catch (AdminException ignored) {
    }

    Time t;
    if((t = u.getDeliveryStartTime()) != null) {
      delivery.setActivePeriodStart(t);
    }

    if((t = u.getDeliveryEndTime()) != null) {
      delivery.setActivePeriodEnd(t);
    }

    delivery.setValidityPeriod(u.getValidityPeriod());
    if(u.getDeliveryDays() != null && !u.getDeliveryDays().isEmpty()) {
      List<Day> days = new ArrayList<Day>(7);
      for (Integer i : u.getDeliveryDays()) {
        days.add(Day.valueOf(i == 0 ? 7 : i));
      }
      delivery.setActiveWeekDays(days.toArray(new Day[days.size()]));
    }
  }

  public void copyUserSettingsToDeliveryPrototype(String user, DeliveryPrototype delivery) throws AdminException {
    User u = getUser(user);
    if (u == null) {
      throw new IntegrityException("user_not_exist", user);
    }
    getDefaultDelivery(u, delivery);
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

  public boolean isRestrictionDaemonStarted() {
    return restrictionDaemon.isStarted();
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

  public Properties getCdrProperties() {
    return webConfig.getCdrProperties();
  }

  public void setCdrProperties(Properties props) throws AdminException {
    cdrDaemon.setCdrOutputDir(new File(props.getProperty(CdrDaemon.CDR_DIR)));
    webConfig.setCdrProperties(props);
  }

  public Properties getSiebelProperties() {
    return webConfig.getSiebelProperties();
  }

  public boolean isAllowUssdPushDeliveries() {
    return webConfig.isAllowUssdPushDeliveries();
  }

  /**
   * Сохраняет настройки Siebel
   * @param props настройик Siebel
   * @return стартовал ли SiebelManager на новых настройках
   * @throws AdminException ошибка валидации или сохранения
   */
  public boolean setSiebelProperties(Properties props) throws AdminException {
    try {
      integrityLock.lock();
      String u = props.getProperty(SiebelManager.USER);
      User user = usersManager.getUser(u);
      if (u == null) {
        throw new UserException("user_not_exist", u);
      }

      Properties old = webConfig.getSiebelProperties();
      if (!old.getProperty(SiebelManager.USER).equals(props.getProperty(SiebelManager.USER))) {
        DeliveryFilter filter = new DeliveryFilter();
        filter.setUserIdFilter(old.getProperty(SiebelManager.USER));
        final boolean[] notExist = new boolean[]{true};
        deliveryManager.getDeliveries(user.getLogin(), user.getPassword(), filter, 1, new Visitor<mobi.eyeline.informer.admin.delivery.Delivery>() {
          public boolean visit(Delivery value) throws AdminException {
            if (value.getStatus() != DeliveryStatus.Finished && value.getProperty(UserDataConsts.SIEBEL_DELIVERY_ID) != null) {
              notExist[0] = false;
              return false;
            }
            return true;
          }
        });
        if (!notExist[0]) {
          throw new SiebelException("can_not_change_user", old.getProperty(SiebelManager.USER));
        }
      }

      boolean siebelStarted = false;
      try {
        siebelFinalStateListener.externalLock();
        siebelFinalStateListener.setPeriodSec(Integer.parseInt(props.getProperty(SiebelFinalStateListener.PERIOD_PARAM)));
        siebelManager.stop();
        try {
          Properties _p = new Properties();
          _p.putAll(props);
          siebelManager.start(user, _p);
          if(!siebelFinalStateListener.isStarted()) {
            siebelFinalStateListener.start();
          }
          siebelStarted = true;
        } catch (Exception e) {
          logger.error("Applying of new properties has failed. Siebel is down.", e);
        }
      } finally {
        siebelFinalStateListener.externalUnlock();
      }
      webConfig.setSiebelProperties(props);
      return siebelStarted;
    } finally {
      integrityLock.unlock();
    }

  }

  protected static class SiebelUserManagerImpl implements SiebelUserManager {

    private AdminContext context;

    public SiebelUserManagerImpl(AdminContext context) {
      this.context = context;
    }

    public User getUser(String login) throws AdminException {
      User u = context.getUser(login);
      if (u == null) {
        throw new IntegrityException("user_not_exist", login);
      }
      return u;
    }
  }


  protected static class SiebelRegionManagerImpl implements SiebelRegionManager {

    private AdminContext context;

    SiebelRegionManagerImpl(AdminContext context) {
      this.context = context;
    }

    public Region getRegion(Address msisdn) throws AdminException {
      return context.getRegion(msisdn);
    }
  }


  protected static class SiebelDeliveriesImpl implements SiebelDeliveries {

    private AdminContext context;

    SiebelDeliveriesImpl(AdminContext context) {
      this.context = context;
    }

    public Delivery createDelivery(String login, String password, DeliveryPrototype delivery, DataSource<Message> msDataSource) throws AdminException {
      return context.createDeliveryWithIndividualTexts(login, password, delivery, msDataSource);
    }

    public void dropDelivery(String login, String password, int deliveryId) throws AdminException {
      context.dropDelivery(login, password, deliveryId);
    }

    public void addMessages(String login, String password, DataSource<Message> msDataSource, int deliveryId) throws AdminException {
      context.addMessages(login, password, msDataSource, deliveryId);
    }

    public Delivery getDelivery(String login, String password, int deliveryId) throws AdminException {
      return context.getDelivery(login, password, deliveryId);
    }

    public void modifyDelivery(String login, String password, Delivery delivery) throws AdminException {
      context.modifyDelivery(login, password, delivery);
    }

    public void cancelDelivery(String login, String password, int deliveryId) throws AdminException {
      context.cancelDelivery(login, password, deliveryId);
    }

    public void pauseDelivery(String login, String password, int deliveryId) throws AdminException {
      context.pauseDelivery(login, password, deliveryId);
    }

    public void activateDelivery(String login, String password, int deliveryId) throws AdminException {
      context.activateDelivery(login, password, deliveryId);
    }

    public void getDeliveries(String login, String password, DeliveryFilter deliveryFilter, int _pieceSize, Visitor<Delivery> visitor) throws AdminException {
      context.getDeliveries(login, password, deliveryFilter, _pieceSize, visitor);
    }

    public void getDefaultDelivery(String user, DeliveryPrototype delivery) throws AdminException {
      context.copyUserSettingsToDeliveryPrototype(user, delivery);
    }
  }

  protected static class ContentProviderContextImpl implements ContentProviderContext{

    private AdminContext context;

    ContentProviderContextImpl(AdminContext context) {
      this.context = context;
    }


    public FileSystem getFileSystem() {
      return this.context.getFileSystem();
    }

    public List<User> getUsers() {
      return this.context.getUsers();
    }

    public User getUser(String userName) {
      return this.context.getUser(userName);  //To change body of implemented methods use File | Settings | File Templates.
    }

    public Region getRegion(Address ab) {
      return this.context.getRegion(ab);
    }

    public Delivery createDelivery(String login, String password, DeliveryPrototype delivery, DataSource<Message> msDataSource) throws AdminException {
      return this.context.createDeliveryWithIndividualTexts(login,password,delivery,msDataSource);
    }

    public Delivery getDelivery(String login, String password, int deliveryId) throws AdminException{
      return this.context.getDelivery(login,password,deliveryId);
    }

    public void activateDelivery(String login, String password, int deliveryId) throws AdminException{
      this.context.activateDelivery(login,password,deliveryId);
    }

    public void dropDelivery(String login, String password, int deliveryId) throws AdminException{
      this.context.dropDelivery(login,password,deliveryId);
    }

    public void copyUserSettingsToDeliveryPrototype(String login, DeliveryPrototype delivery) throws AdminException{
      this.context.copyUserSettingsToDeliveryPrototype(login,delivery);
    }

    public void addMessages(String login, String password, DataSource<Message> messageSource, int deliveryId) throws AdminException{
      this.context.addMessages(login,password,messageSource,deliveryId);
    }

    public void getMessagesStates(String login, String password, MessageFilter filter, int deliveryId, Visitor<Message> visitor) throws AdminException{
      this.context.getMessagesStates(login,password,filter,deliveryId,visitor);
    }
  }

  protected static class  DeliveryNotificationsContextImpl implements DeliveryNotificationsContext {
    private AdminContext context;

    DeliveryNotificationsContextImpl(AdminContext context) {
      this.context = context;
    }

    public User getUser(String userId) throws AdminException {
      return context.getUser(userId);
    }

    public Delivery getDelivery(String login, String password, int deliveryId) throws AdminException {
      return context.getDelivery(login,password,deliveryId);
    }

    public Address getSmsSenderAddress() throws AdminException {
      return context.getSmsSenderAddress();
    }

    public Properties getNotificationTemplates() {
      return context.getNotificationTemplates();
    }

    public void sendTestSms(TestSms testSms) throws AdminException {
      context.sendTestSms(testSms);
    }

    public Properties getJavaMailProperties() {
      return context.getJavaMailProperties();
    }
  }

  protected static class CdrUsersImpl implements CdrUsers {

    private AdminContext context;

    public CdrUsersImpl(AdminContext context) {
      this.context = context;
    }

    public User getUser(String login) {
      return context.getUser(login);
    }
  }

  protected static class CdrDeliveriesImpl implements CdrDeliveries {

    private AdminContext context;

    public CdrDeliveriesImpl(AdminContext context) {
      this.context = context;
    }

    public Delivery getDelviery(String user, int deliveryId) throws AdminException {
      User u = context.getUser(user);
      if(u == null) {
        throw new UserException("user_not_exist");
      }
      return context.getDelivery(user, u.getPassword(), deliveryId);
    }
  }

}
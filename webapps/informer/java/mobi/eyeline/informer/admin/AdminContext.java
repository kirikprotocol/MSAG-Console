package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.cdr.CdrProvider;
import mobi.eyeline.informer.admin.cdr.CdrSettings;
import mobi.eyeline.informer.admin.contentprovider.FileDeliveriesProvider;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangesDetectorImpl;
import mobi.eyeline.informer.admin.delivery.stat.*;
import mobi.eyeline.informer.admin.dep.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.informer.InformerSettings;
import mobi.eyeline.informer.admin.infosme.TestSms;
import mobi.eyeline.informer.admin.journal.Journal;
import mobi.eyeline.informer.admin.notifications.DateAndFile;
import mobi.eyeline.informer.admin.notifications.DeliveryNotificationsDaemon;
import mobi.eyeline.informer.admin.notifications.NotificationSettings;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.restriction.Restriction;
import mobi.eyeline.informer.admin.restriction.RestrictionDaemon;
import mobi.eyeline.informer.admin.restriction.RestrictionsFilter;
import mobi.eyeline.informer.admin.siebel.SiebelException;
import mobi.eyeline.informer.admin.siebel.SiebelManager;
import mobi.eyeline.informer.admin.siebel.SiebelSettings;
import mobi.eyeline.informer.admin.siebel.impl.SiebelDeliveries;
import mobi.eyeline.informer.admin.siebel.impl.SiebelFinalStateListener;
import mobi.eyeline.informer.admin.siebel.impl.SiebelRegionManager;
import mobi.eyeline.informer.admin.siebel.impl.SiebelUserManager;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;

import java.io.File;
import java.util.*;

/**
 * Класс для управления моделью
 *
 * @author Aleksandr Khalitov
 */
public class AdminContext extends AdminContextBase {


  protected User2RegionDep user2regionDep;
  protected Delivery2UserDep delivery2UserDep;
  protected Restriction2UserDep restriction2UserDep;
  protected Region2SmscDep region2smscDep;

  protected RestrictionDaemon restrictionDaemon;
  protected DeliveryNotificationsDaemon deliveryNotificationsDaemon;
  protected CdrProvider cdrProvider;
  protected FileDeliveriesProvider fileDeliveriesProvider;

  public AdminContext() {
  }

  public AdminContext(File appBaseDir) throws InitException {
    super(appBaseDir);

    try {
      restrictionDaemon = new RestrictionDaemon(new RestrictionDaemonContextImpl(this));

      fileDeliveriesProvider = new FileDeliveriesProvider(new ContentProviderContextImpl(this, deliveryChangesDetector), appBaseDir, workDir, webConfig.getContentProviderPeriod());

      deliveryNotificationsDaemon = new DeliveryNotificationsDaemon(new DeliveryNotificationsContextImpl(this));

      deliveryChangesDetector.addListener(deliveryNotificationsDaemon);

      restrictionDaemon.start();

      InformerSettings is = informerManager.getConfigSettings();
      deliveryChangesDetector = new DeliveryChangesDetectorImpl(new File(is.getStoreDir(), "final_log"), fileSystem);

      try {
        initSiebel(workDir);
      } catch (Exception e) {
        logger.error(e, e);
      }

      cdrProvider = new CdrProvider(new CdrProviderContextImpl(this, deliveryChangesDetector), webConfig.getCdrSettings(), new File(workDir, "cdr"), fileSystem);

      deliveryChangesDetector.start();

    } catch (AdminException e) {
      throw new InitException(e);
    }

    initDependencies();
  }

  protected void initDependencies() {
    user2regionDep = new User2RegionDep(usersManager, regionsManager);
    delivery2UserDep = new Delivery2UserDep(deliveryManager, usersManager);
    restriction2UserDep = new Restriction2UserDep(restrictionsManager, restrictionDaemon, usersManager);
    region2smscDep = new Region2SmscDep(regionsManager, smscManager);
  }

  protected void initSiebel(File workDir) throws AdminException, InitException {

    SiebelDeliveries siebelDeliveries = new SiebelDeliveriesImpl(this);
    SiebelRegionManager siebelRegions = new SiebelRegionManagerImpl(this);
    SiebelUserManager userManager = new SiebelUserManagerImpl(this);

    User siebelUser = usersManager.getUser(webConfig.getSiebelSettings().getUser());

    if (siebelUser == null) {
      throw new IntegrityException("user_not_exist", webConfig.getSiebelSettings().getUser());
    }

    siebelManager = new SiebelManager(siebelDeliveries, siebelRegions);

    siebelFinalStateListener = new SiebelFinalStateListener(siebelManager, siebelDeliveries,
        userManager, workDir,
        webConfig.getSiebelSettings().getStatsPeriod());

    deliveryChangesDetector.addListener(siebelFinalStateListener);

    siebelManager.start(siebelUser, webConfig.getSiebelSettings());

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

  public void shutdown() {
    shutdownSiebel();
    cdrProvider.shutdown();
    fileDeliveriesProvider.shutdown();

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

    super.shutdown();
  }



  @Deprecated
  public boolean isCdrStarted() {
    return true;
  }

  public File getWorkDir() {
    return new File(workDir.getAbsolutePath());
  }



  public Journal getJournal() {
    return journal;
  }

  public FileSystem getFileSystem() {
    return fileSystem;
  }

  // USERS =======================================================================================

  public User getUser(String login) {
    return usersManager.getUser(login);
  }

  public List<User> getUsers() {
    return usersManager.getUsers();
  }

  public synchronized void updateUser(User u) throws AdminException {
    user2regionDep.updateUser(u);
    usersManager.updateUser(u);
    restriction2UserDep.updateUser(u);
  }

  public synchronized void addUser(User u) throws AdminException {
    user2regionDep.updateUser(u);
    usersManager.addUser(u);
  }

  public synchronized void removeUser(String login) throws AdminException {
    if (login.equals(webConfig.getSiebelSettings().getUser()))
      throw new IntegrityException("fail.delete.user.siebel", login);

    User user = usersManager.getUser(login);
    delivery2UserDep.removeUser(user);
    restriction2UserDep.removeUser(user);

    usersManager.removeUser(login);
  }

  // INFORMER CONFIG ==================================================================================

  public InformerSettings getConfigSettings() throws AdminException {
    return informerManager.getConfigSettings();
  }

  public void updateConfigSettings(InformerSettings informerSettings) throws AdminException {
    informerManager.updateSettings(informerSettings);
  }

  // BLACK LIST  =======================================================================================

  public boolean isBlackListEnabled() throws AdminException {
    return informerManager.getConfigSettings().isPvssEnabledBL();
  }

  public void addInBlacklist(Address msisdn) throws AdminException {
    if(!isBlackListEnabled()) {
      throw new IntegrityException("blacklist.disabled");
    }
    blacklistManager.add(msisdn);
  }

  public void addInBlacklist(Collection<Address> msisdns) throws AdminException {
    if(!isBlackListEnabled()) {
      throw new IntegrityException("blacklist.disabled");
    }
    blacklistManager.add(msisdns);
  }

  public void removeFromBlacklist(Address msisdn) throws AdminException {
    if(!isBlackListEnabled()) {
      throw new IntegrityException("blacklist.disabled");
    }
    blacklistManager.remove(msisdn);
  }

  public void removeFromBlacklist(Collection<Address> msisdns) throws AdminException {
    if(!isBlackListEnabled()) {
      throw new IntegrityException("blacklist.disabled");
    }
    blacklistManager.remove(msisdns);
  }

  public boolean blacklistContains(Address msisdn) throws AdminException {
    if(!isBlackListEnabled()) {
      throw new IntegrityException("blacklist.disabled");
    }
    return blacklistManager.contains(msisdn);
  }

  // SMSC =======================================================================================

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

  public synchronized void removeSmsc(String smscName) throws AdminException {
    region2smscDep.removeSmsc(smscName);
    smscManager.removeSmsc(smscName);
  }

  public void setDefaultSmsc(String smsc) throws AdminException {
    smscManager.setDefaultSmsc(smsc);
  }

  public String getDefaultSmsc() {
    return smscManager.getDefaultSmsc();
  }

  // REGIONS =======================================================================================

  public synchronized void updateRegionsConfiguration(Collection<Region> regions) throws AdminException {
    for (Region region : regions)
      region2smscDep.updateRegion(region);

    regionsManager.checkRegionsConfiguration(regions);

    Collection<Region> toAdd = new ArrayList<Region>();
    Collection<Region> toUpdate = new ArrayList<Region>();
    Collection<Region> toRemove = new ArrayList<Region>();

    toRemove.addAll(regionsManager.getRegions());

    for (Region r : regions) {
      Region existedR = regionsManager.getRegionByName(r.getName());
      if (existedR != null) {
        toRemove.remove(existedR);
        existedR.setSmsc(r.getSmsc());
        existedR.clearMasks();
        existedR.addMasks(r.getMasks());
        existedR.setMaxSmsPerSecond(r.getMaxSmsPerSecond());
        toUpdate.add(existedR);

      } else
        toAdd.add(r);
    }

    for (Region r : toRemove)
      removeRegion(r.getRegionId());

    for (Region r : toUpdate)
      updateRegion(r);

    for (Region r : toAdd)
      addRegion(r);
  }

  public synchronized void addRegion(Region region) throws AdminException {
    region2smscDep.updateRegion(region);
    regionsManager.addRegion(region);
  }

  public synchronized void updateRegion(Region region) throws AdminException {
    region2smscDep.updateRegion(region);
    regionsManager.updateRegion(region);
  }

  public int getDefaultMaxPerSecond() {
    return regionsManager.getDefaultMaxPerSecond();
  }

  public void setDefaultMaxPerSecond(int defMaxPerSecond) throws AdminException {
    regionsManager.setDefaultMaxPerSecond(defMaxPerSecond);
  }

  public synchronized void removeRegion(Integer regionId) throws AdminException {
    user2regionDep.removeRegion(regionId);
    regionsManager.removeRegion(regionId);
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

  // INFORMER =============================================================================================================================

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

  public void statisticByUsers(UserStatFilter filter, UserStatVisitor visitor) throws AdminException {
    deliveryManager.statisticsByUser(filter, visitor);
  }

  public void getStatEntities(StatEntityProvider.EntityVisitor v, Date from, Date till) throws AdminException {
    deliveryManager.getStatEntities(v, from, till);
  }

  public void dropStatEntities(Date from, Date till) throws AdminException {
    deliveryManager.dropStatEntities(from, till);
  }

  public List<DateAndFile> getProcessedNotificationsFiles(Date startDate,Date endDate) throws AdminException {
    return deliveryChangesDetector.getProcessedNotificationsFiles(startDate,endDate);
  }


  public List<Daemon> getDaemons() {
    List<Daemon> ret = new LinkedList<Daemon>();
    ret.add(restrictionDaemon);
    return ret;
  }

  public boolean isSiebelDaemonStarted() {
    return siebelManager.isStarted();
  }

  // DELIVERIES ====================================================================================================================

  public synchronized Delivery createDeliveryWithIndividualTexts(String login, String password, DeliveryPrototype delivery, DataSource<Message> msDataSource) throws AdminException {
    if (restrictionsManager.hasActiveRestriction(login)) {
      throw new DeliveryException("creation_restricted");
    }

    delivery2UserDep.checkDelivery(delivery);

    User u = usersManager.getUser(delivery.getOwner());
    if(u.isCreateCDR()) {
      delivery.setEnableMsgFinalizationLogging(true);
    }
    return deliveryManager.createDeliveryWithIndividualTexts(login, password, delivery, msDataSource);
  }

  public synchronized Delivery createDeliveryWithSingleText(String login, String password, DeliveryPrototype delivery, DataSource<Address> msDataSource) throws AdminException {
    if (restrictionsManager.hasActiveRestriction(login)) {
      throw new DeliveryException("creation_restricted");
    }

    delivery2UserDep.checkDelivery(delivery);
    User u = usersManager.getUser(delivery.getOwner());

    if(u.isCreateCDR()) {
      delivery.setEnableMsgFinalizationLogging(true);
    }
    return deliveryManager.createDeliveryWithSingleText(login, password, delivery, msDataSource);
  }

  public synchronized void modifyDelivery(String login, String password, Delivery delivery) throws AdminException {
    delivery2UserDep.checkDelivery(delivery);

    synchronized (getLock(delivery.getId())) {
      deliveryManager.modifyDelivery(login, password, delivery);
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
      Delivery d = deliveryManager.getDelivery(login, password, deliveryId);
      if(d.getProperty(UserDataConsts.SIEBEL_DELIVERY_ID) != null) {
        throw new IntegrityException("siebel.delivery.remove");
      }
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
    if (u.getEmail() != null && u.isEmailNotification()) {
      delivery.setProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS, u.getEmail());
    }
    if (u.getPhone() != null && u.isSmsNotification()) {
      delivery.setProperty(UserDataConsts.SMS_NOTIF_ADDRESS, u.getPhone().getSimpleAddress());
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

    delivery.setPriority(u.getPriority());
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

  // RESTRICTIONS ================================================================================================================================

  public Restriction getRestriction(int id) {
    return restrictionsManager.getRestriction(id);
  }

  public List<Restriction> getRestrictions(RestrictionsFilter filter) {
    return restrictionsManager.getRestrictions(filter);
  }

  public synchronized void addRestriction(Restriction r) throws AdminException {
    restriction2UserDep.updateRestriction(r);
    restrictionsManager.addRestriction(r);
    restrictionDaemon.rebuildSchedule();
  }

  public synchronized void updateRestriction(Restriction r) throws AdminException {
    restriction2UserDep.updateRestriction(r);
    restrictionsManager.updateRestriction(r);
    restrictionDaemon.rebuildSchedule();
  }

  public synchronized void deleteRestriction(int id) throws AdminException {
    restrictionsManager.deleteRestriction(id);
    restrictionDaemon.rebuildSchedule();
  }

  public boolean isRestrictionDaemonStarted() {
    return restrictionDaemon.isStarted();
  }


  public NotificationSettings getNotificationSettings() {
    return webConfig.getNotificationSettings();
  }

  public void setNotificationSettings(NotificationSettings props) throws AdminException {
    webConfig.setNotificationSettings(props);
  }

  public CdrSettings getCdrSettings() {
    return webConfig.getCdrSettings();
  }

  public void setCdrSettings(CdrSettings props) throws AdminException {
    webConfig.setCdrSettings(props);
    cdrProvider.updateSettings(props);
  }

  public boolean isAllowUssdPushDeliveries() {
    return webConfig.isAllowUssdPushDeliveries();
  }

  // SIEBEL =========================================================================================================

  public SiebelSettings getSiebelSettings() {
    return webConfig.getSiebelSettings();
  }

  public boolean checkSiebelSettings(SiebelSettings p) throws AdminException{
    p.validate();
    if(siebelManager != null) {
      try{
        siebelManager.checkProperties(p);
        return true;
      }catch (AdminException ignored) {}
    }
    return false;
  }

  /**
   * Сохраняет настройки Siebel
   * @param siebelSettings настройик Siebel
   * @return стартовал ли SiebelManager на новых настройках
   * @throws AdminException ошибка валидации или сохранения
   */
  public synchronized boolean setSiebelSettings(SiebelSettings siebelSettings) throws AdminException {

    String u = siebelSettings.getUser();
    User user = usersManager.getUser(u);

    SiebelSettings old = webConfig.getSiebelSettings();
    if (!old.getUser().equals(siebelSettings.getUser())) {
      DeliveryFilter filter = new DeliveryFilter();
      filter.setUserIdFilter(old.getUser());
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
        throw new SiebelException("can_not_change_user", old.getUser());
      }
    }


    boolean siebelStarted = false;
    try {
      siebelFinalStateListener.externalLock();
      webConfig.setSiebelSettings(siebelSettings);
      siebelFinalStateListener.setPeriodSec(siebelSettings.getStatsPeriod());
      siebelManager.stop();
      try {
        SiebelSettings _p = new SiebelSettings(siebelSettings);
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
    return siebelStarted;
  }

  public void verifyCPSettings(User u, UserCPsettings ucps) throws AdminException {
    fileDeliveriesProvider.verifyConnection(u, ucps);
  }

  public void sendTestSms(TestSms sms) throws AdminException {
    infosme.sendTestSms(sms);
  }

  public void sendTestEmailNotification(User user, String email, NotificationSettings settings) throws AdminException {
    deliveryNotificationsDaemon.sendTestEmailNotification(user, email, settings);
  }
  public void sendTestSMSNotification(User user, Address address, DeliveryStatus status, NotificationSettings settings) throws AdminException {
    deliveryNotificationsDaemon.sendTestSMSNotification(user,address,status, settings);
  }

  public void validateDeliveryWithIndividualTexts(DeliveryPrototype delivery) throws AdminException {
    DeliveryManager.validateDeliveryWithIndividualTexts(delivery);
  }

  public void validateDeliveryWithSingleText(DeliveryPrototype delivery) throws AdminException {
    DeliveryManager.validateDeliveryWithSingleText(delivery);
  }

}
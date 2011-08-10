package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.archive.*;
import mobi.eyeline.informer.admin.cdr.CdrProvider;
import mobi.eyeline.informer.admin.cdr.CdrProviderContext;
import mobi.eyeline.informer.admin.cdr.CdrSettings;
import mobi.eyeline.informer.admin.contentprovider.ContentProviderContext;
import mobi.eyeline.informer.admin.contentprovider.FileDeliveriesProvider;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangesDetector;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangesDetectorImpl;
import mobi.eyeline.informer.admin.delivery.stat.*;
import mobi.eyeline.informer.admin.dep.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.ftpserver.FtpUser;
import mobi.eyeline.informer.admin.informer.InformerSettings;
import mobi.eyeline.informer.admin.infosme.TestSms;
import mobi.eyeline.informer.admin.journal.Journal;
import mobi.eyeline.informer.admin.monitoring.MonitoringEventsFilter;
import mobi.eyeline.informer.admin.monitoring.MonitoringJournal;
import mobi.eyeline.informer.admin.notifications.DeliveryNotificationsContext;
import mobi.eyeline.informer.admin.notifications.DeliveryNotificationsProvider;
import mobi.eyeline.informer.admin.notifications.NotificationSettings;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.restriction.Restriction;
import mobi.eyeline.informer.admin.restriction.RestrictionContext;
import mobi.eyeline.informer.admin.restriction.RestrictionProvider;
import mobi.eyeline.informer.admin.restriction.RestrictionsFilter;
import mobi.eyeline.informer.admin.service.ServiceInfo;
import mobi.eyeline.informer.admin.siebel.SiebelContext;
import mobi.eyeline.informer.admin.siebel.SiebelProvider;
import mobi.eyeline.informer.admin.siebel.SiebelSettings;
import mobi.eyeline.informer.admin.smppgw.*;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.DateAndFile;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Date;
import java.util.List;

/**
 * Класс для управления моделью
 *
 * @author Aleksandr Khalitov
 */
public class AdminContext extends AdminContextBase implements CdrProviderContext, ContentProviderContext, DeliveryNotificationsContext, SiebelContext, RestrictionContext, ArchiveContext, SmppGWConfigManagerContext {

  protected User2RegionDep user2regionDep;
  protected Delivery2UserDep delivery2UserDep;
  protected Restriction2UserDep restriction2UserDep;
  protected Region2SmscDep region2smscDep;
  protected SmppGW2DeliveryDep smppGW2DeliveryDep;
  protected SmppGW2UserDep smppGW2UserDep;

  protected RestrictionProvider restrictionProvider;
  protected DeliveryNotificationsProvider deliveryNotificationsProvider;
  protected CdrProvider cdrProvider;
  protected FileDeliveriesProvider fileDeliveriesProvider;
  protected SiebelProvider siebelProvider;

  protected ArchiveRequestsManager archiveRequestsManager;

  protected DeliveryManager archiveDeliveryManager;

  protected SmppGWServiceManager smppGWServiceManager;

  protected SmppGWConfigManager smppGWManager;

  private SmppGW smppGW;

  public AdminContext() {
  }

  public AdminContext(File appBaseDir) throws InitException {
    super(appBaseDir);

    try {
      File servicesDir = new File(appBaseDir, "services");

      File confDir = new File(servicesDir, "Informer" + File.separatorChar + "conf");

      restrictionProvider = new RestrictionProvider(this, new File(confDir, "restrictions.csv"), new File(confDir, "backup"), fileSystem);

      InformerSettings is = informerManager.getConfigSettings();
      deliveryChangesDetector = new DeliveryChangesDetectorImpl(new File(is.getStoreDir(), "final_log"), fileSystem);

      fileDeliveriesProvider = new FileDeliveriesProvider(this, appBaseDir, workDir);

      deliveryNotificationsProvider = new DeliveryNotificationsProvider(this, webConfig.getNotificationSettings());

      siebelProvider = new SiebelProvider(this, webConfig.getSiebelSettings(), workDir);

      cdrProvider = new CdrProvider(this, webConfig.getCdrSettings(), new File(workDir, "cdr"), fileSystem);

      deliveryChangesDetector.start();

      if(archiveDaemonManager != null) {
        ArchiveDaemonSettings archiveDaemonSettings = archiveDaemonManager.getSettings();
        archiveDeliveryManager = new DeliveryManager(archiveDaemonSettings.getDcpHost(), archiveDaemonSettings.getDcpPort(), workDir, fileSystem);
        archiveRequestsManager = new ArchiveRequestsManager(this, this.webConfig.getArchiveSettings());
      }
      ServiceInfo si = serviceManager.getService(SmppGWServiceManager.SERVICE_ID);
      if (si != null) {
        smppGWServiceManager = new SmppGWServiceManager(serviceManager);
        File configDir = new File(si.getBaseDir().getAbsolutePath()+File.separatorChar+"conf");
        smppGW = new SmppGWImpl(new File(configDir, "config.properties"), fileSystem);
        smppGWManager = new SmppGWConfigManager(smppGW, configDir, new File(configDir, "backup") , this);
      }

    } catch (AdminException e) {
      throw new InitException(e);
    }

    initDependencies();
  }

  protected void initDependencies() {
    user2regionDep = new User2RegionDep(usersManager, regionsManager);
    delivery2UserDep = new Delivery2UserDep(deliveryManager, usersManager);
    restriction2UserDep = new Restriction2UserDep(restrictionProvider, usersManager);
    region2smscDep = new Region2SmscDep(regionsManager, smscManager);
    if(smppGWManager != null) {
      smppGW2DeliveryDep = new SmppGW2DeliveryDep(smppGWManager);
      smppGW2UserDep = new SmppGW2UserDep(smppGWManager);
    }
  }

  public void shutdown() {
    siebelProvider.shutdown();
    cdrProvider.shutdown();
    fileDeliveriesProvider.shutdown();
    restrictionProvider.shutdown();
    deliveryNotificationsProvider.shutdown();

    try {
      deliveryChangesDetector.shutdown();
    } catch (Exception ignored) {
    }

    if(archiveRequestsManager != null) {
      archiveRequestsManager.shutdown();
    }
    if(smppGW != null) {
      smppGW.shutdown();
    }

    super.shutdown();
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

  @Override
  public DeliveryChangesDetector getDeliveryChangesDetector() {
    return deliveryChangesDetector;
  }

  @Override
  public File getFtpUserHomeDir(String login) throws AdminException {
    if (ftpServerManager != null)
      return ftpServerManager.getUserHomeDir(login);
    return null;
  }

  public String getFtpServerOnlineHost() throws AdminException {
    if (!isFtpServerDeployed())
      return null;
    return ftpServerManager.getOnlineHost();
  }

  public List<String> getFtpServerHosts() throws AdminException {
    if (!isFtpServerDeployed())
      return null;
    return ftpServerManager.getServerHosts();
  }

  public void startFtpServer() throws AdminException {
    if (isFtpServerDeployed())
      ftpServerManager.startServer();
  }

  public void stopFtpServer() throws AdminException {
    if (isFtpServerDeployed())
      ftpServerManager.stopServer();
  }

  public void switchFtpServer(String toHost) throws AdminException {
    if (isFtpServerDeployed())
      ftpServerManager.switchServer(toHost);
  }

  // USERS =======================================================================================

  public User getUser(String login) {
    return usersManager.getUser(login);
  }

  @Override
  public boolean containsDelivery(String login, int deliveryId) throws AdminException {
    return getDelivery(login, deliveryId) != null;
  }

  @Override
  public UnmodifiableDeliveryManager getDeliveryManager() {
    return archiveDeliveryManager;
  }

  public List<User> getUsers() {
    return usersManager.getUsers();
  }

  private void updateLocalFtpAccounts() throws AdminException {
    if (!isFtpServerDeployed())
      return;

    List<FtpUser> ftpUsers = new ArrayList<FtpUser>();
    for (User u : getUsers()) {
      if (u.getCpSettings() == null)
        continue;
      for (UserCPsettings s : u.getCpSettings()) {
        if (s.getProtocol() == UserCPsettings.Protocol.localFtp) {
          ftpUsers.add(new FtpUser(s.getLogin(), s.getPassword(), s.getDirectoryMaxSize()));
        }
      }
    }
    try{
      ftpServerManager.updateUsers(ftpUsers);
    }catch (AdminException e){
      logger.error(e,e);
    }
  }

  public synchronized void updateUser(User u) throws AdminException {
    user2regionDep.updateUser(u);
    usersManager.updateUser(u);
    if(smppGW2UserDep != null) {
      smppGW2UserDep.updateUser(u);
    }
    updateLocalFtpAccounts();
    restriction2UserDep.updateUser(u);
  }

  public synchronized void addUser(User u) throws AdminException {
    user2regionDep.updateUser(u);
    usersManager.addUser(u);
    updateLocalFtpAccounts();
  }

  public synchronized void removeUser(String login) throws AdminException {
    if (login.equals(webConfig.getSiebelSettings().getUser()))
      throw new IntegrityException("fail.delete.user.siebel", login);

    User user = usersManager.getUser(login);
    delivery2UserDep.removeUser(user);
    restriction2UserDep.removeUser(user);

    usersManager.removeUser(login);
    updateLocalFtpAccounts();
  }

  // INFORMER CONFIG ==================================================================================

  public InformerSettings getConfigSettings() throws AdminException {
    return informerManager.getConfigSettings();
  }

  public void updateConfigSettings(InformerSettings informerSettings) throws AdminException {
    informerManager.updateSettings(informerSettings);
  }

  public boolean isAllowUssdPushDeliveries() {
    return webConfig.isAllowUssdPushDeliveries();
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

  // STATISTICS ====================================================================================================================

  public void statistics(DeliveryStatFilter filter, DeliveryStatVisitor visitor) throws AdminException {
    statisticsManager.statistics(filter, visitor);
  }

  public void statisticByUsers(UserStatFilter filter, UserStatVisitor visitor) throws AdminException {
    statisticsManager.statisticsByUser(filter, visitor);
  }

  public void getStatEntities(StatEntityProvider.EntityVisitor v, Date from, Date till) throws AdminException {
    statisticsManager.getStatEntities(v, from, till);
  }

  public void dropStatEntities(Date from, Date till) throws AdminException {
    statisticsManager.dropStatEntities(from, till);
  }

  public List<DateAndFile> getProcessedNotificationsFiles(Date startDate,Date endDate) throws AdminException {
    return deliveryChangesDetector.getProcessedNotificationsFiles(startDate, endDate);
  }

  // ARCHIVE =======================================================================================================================




  // DELIVERIES ====================================================================================================================

  private void checkHasNotRestriction(String errorMessage, String login) throws AdminException {
    RestrictionsFilter filter = new RestrictionsFilter();
    Date startDate = new Date();
    filter.setUserId(login);
    filter.setStartDate(startDate);
    filter.setEndDate(startDate);
    List<Restriction> rs = restrictionProvider.getRestrictions(filter);
    if(!rs.isEmpty()) {
      Restriction r = rs.get(0);
      SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yyyy HH:mm");
      throw new DeliveryException(errorMessage, sdf.format(r.getStartDate()), sdf.format(r.getEndDate()), r.getName());
    }
  }


  public Delivery createDeliveryWithIndividualTexts(String login, DeliveryPrototype delivery, DataSource<Message> msDataSource) throws AdminException {
    checkHasNotRestriction("creation_restricted", login);

    delivery2UserDep.checkDelivery(delivery);

    User u = getUser(delivery.getOwner());
    if(u.isCreateCDR()) {
      delivery.setEnableMsgFinalizationLogging(true);
    }
    if (delivery.getProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS) != null || delivery.getProperty(UserDataConsts.SMS_NOTIF_ADDRESS) != null)
      delivery.setEnableStateChangeLogging(true);
    return deliveryManager.createDeliveryWithIndividualTexts(login, u.getPassword(), delivery, msDataSource);
  }


  public Delivery createDeliveryWithSingleTextWithData(String login, DeliveryPrototype delivery, DataSource<Message> msDataSource) throws AdminException {
    checkHasNotRestriction("creation_restricted", login);

    delivery2UserDep.checkDelivery(delivery);

    User u = getUser(login);

    if(u.isCreateCDR()) {
      delivery.setEnableMsgFinalizationLogging(true);
    }
    if (delivery.getProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS) != null || delivery.getProperty(UserDataConsts.SMS_NOTIF_ADDRESS) != null)
      delivery.setEnableStateChangeLogging(true);
    return deliveryManager.createDeliveryWithSingleTextWithData(login, u.getPassword(), delivery, msDataSource);
  }

  public Delivery createDeliveryWithSingleText(String login, DeliveryPrototype delivery, DataSource<Address> msDataSource) throws AdminException {
    checkHasNotRestriction("creation_restricted", login);

    delivery2UserDep.checkDelivery(delivery);
    User u = usersManager.getUser(delivery.getOwner());

    if(u.isCreateCDR()) {
      delivery.setEnableMsgFinalizationLogging(true);
    }
    if (delivery.getProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS) != null || delivery.getProperty(UserDataConsts.SMS_NOTIF_ADDRESS) != null)
      delivery.setEnableStateChangeLogging(true);
    return deliveryManager.createDeliveryWithSingleText(login, u.getPassword(), delivery, msDataSource);
  }

  @Override
  public void checkNoRestrictions(String login) throws AdminException {
    checkHasNotRestriction("creation_restricted", login);
  }

  public void modifyDelivery(String login, Delivery delivery) throws AdminException {
    delivery2UserDep.checkDelivery(delivery);

    User u =  usersManager.getUser(login);

    try {
      lockDelivery(delivery.getId());
      deliveryManager.modifyDelivery(login, u.getPassword(), delivery);
    } finally {
      unlockDelivery(delivery.getId());
    }
  }

  public Delivery setDeliveryRestriction(String login, int deliveryId, boolean restriction) throws AdminException {
    User user = getUser(login);
    try {
      lockDelivery(deliveryId);
      Delivery d = deliveryManager.getDelivery(login, user.getPassword(), deliveryId);
      d.setProperty(UserDataConsts.RESTRICTION, Boolean.toString(restriction));
      deliveryManager.modifyDelivery(login, user.getPassword(), d);
      return d;
    } finally {
      unlockDelivery(deliveryId);
    }
  }

  public void archivateDelivery(String login, int deliveryId) throws AdminException {
    if(smppGW2DeliveryDep != null) {
      smppGW2DeliveryDep.archivateDelivery(deliveryId);
    }
    if(!isArchiveDaemonDeployed()) {
      throw new IntegrityException("archiveDaemon.undeployed");
    }
    User u = getUser(login);
    try {
      lockDelivery(deliveryId);
      Delivery d = deliveryManager.getDelivery(login, u.getPassword(), deliveryId);
      if(d.getProperty(UserDataConsts.SIEBEL_DELIVERY_ID) != null) {
        throw new IntegrityException("siebel.delivery.remove");
      }
      setDeliveryRestriction(login, deliveryId, false);
      deliveryManager.archivateDelivery(login, u.getPassword(), deliveryId);
    } finally {
      unlockDelivery(deliveryId);
    }
  }

  public void dropDelivery(String login, int deliveryId) throws AdminException {
    if(smppGW2DeliveryDep != null) {
      smppGW2DeliveryDep.removeDelivery(deliveryId);
    }
    User user = getUser(login);
    try {
      lockDelivery(deliveryId);
      Delivery d = deliveryManager.getDelivery(login, user.getPassword(), deliveryId);
      if(d == null) {
        return;
      }
      if(d.getProperty(UserDataConsts.SIEBEL_DELIVERY_ID) != null && !d.getOwner().equals(login)) {
        throw new IntegrityException("siebel.delivery.remove");
      }
      setDeliveryRestriction(login, deliveryId, false);
      deliveryManager.dropDelivery(login, user.getPassword(), deliveryId);
    } finally {
      unlockDelivery(deliveryId);
    }
  }

  @Override
  public void addMessages(String login, DataSource<Message> messageSource, int deliveryId) throws AdminException {
    User u = getUser(login);
    addMessages(login, u.getPassword(), messageSource, deliveryId);
  }

  public void addSingleMessagesWithData(String login, DataSource<Message> messageSource, int deliveryId) throws AdminException {
    User u = getUser(login);
    try {
      lockDelivery(deliveryId);
      deliveryManager.addSingleTextMessagesWithData(login, u.getPassword(), messageSource, deliveryId);
    } finally {
      unlockDelivery(deliveryId);
    }
  }

  public void addMessages(String login, String password, DataSource<Message> msDataSource, int deliveryId) throws AdminException {
    try {
      lockDelivery(deliveryId);
      deliveryManager.addIndividualMessages(login, password, msDataSource, deliveryId);
    } finally {
      unlockDelivery(deliveryId);
    }
  }

  public void addSingleTextMessages(String login, DataSource<Address> msDataSource, int deliveryId) throws AdminException {
    User u = getUser(login);
    try {
      lockDelivery(deliveryId);
      deliveryManager.addSingleTextMessages(login, u.getPassword(), msDataSource, deliveryId);
    } finally {
      unlockDelivery(deliveryId);
    }
  }

  public int countDeliveries(String login, DeliveryFilter deliveryFilter) throws AdminException {
    User u = getUser(login);
    return deliveryManager.countDeliveries(login, u.getPassword(), deliveryFilter);
  }

  public Delivery getDelivery(String login, int deliveryId) throws AdminException {
    User u = getUser(login);
    return deliveryManager.getDelivery(login, u.getPassword(), deliveryId);
  }

  public void cancelDelivery(String login, int deliveryId) throws AdminException {
    User u = getUser(login);
    try {
      lockDelivery(deliveryId);
      setDeliveryRestriction(login, deliveryId, false);
      deliveryManager.cancelDelivery(login, u.getPassword(), deliveryId);
    } finally {
      unlockDelivery(deliveryId);
    }
  }

  public void pauseDelivery(String login, int deliveryId) throws AdminException {
    User u = getUser(login);
    try {
      lockDelivery(deliveryId);
      deliveryManager.pauseDelivery(login, u.getPassword(), deliveryId);
    } finally {
      unlockDelivery(deliveryId);
    }
  }


  public void activateDelivery(String login, int deliveryId) throws AdminException {
    User u = getUser(login);
    try {
      lockDelivery(deliveryId);
      checkHasNotRestriction("activation_restricted", login);
      deliveryManager.activateDelivery(login, u.getPassword(), deliveryId);
    } finally {
      unlockDelivery(deliveryId);
    }
  }

  public DeliveryStatistics getDeliveryStats(String login, int deliveryId) throws AdminException {
    User u = getUser(login);
    return deliveryManager.getDeliveryStats(login, u.getPassword(), deliveryId);
  }

  public void getDeliveries(String login, DeliveryFilter deliveryFilter, Visitor<Delivery> visitor) throws AdminException {
    getDeliveries(login, deliveryFilter, 1000, visitor);
  }

  public void getDeliveries(String login, DeliveryFilter deliveryFilter, int pieceSize, Visitor<Delivery> visitor) throws AdminException {
    User u = getUser(login);
    deliveryManager.getDeliveries(login, u.getPassword(), deliveryFilter, pieceSize, visitor);
  }

  public void getMessagesStates(String login, MessageFilter filter, int _pieceSize, Visitor<Message> visitor) throws AdminException {
    User u = getUser(login);
    deliveryManager.getMessages(login, u.getPassword(), filter, _pieceSize, visitor);
  }

  public int countMessages(String login, MessageFilter messageFilter) throws AdminException {
    User u = getUser(login);
    return deliveryManager.countMessages(login, u.getPassword(), messageFilter);
  }

  public DeliveryStatusHistory getDeliveryStatusHistory(String login, int deliveryId) throws AdminException {
    User u = getUser(login);
    return deliveryManager.getDeliveryStatusHistory(login, u.getPassword(), deliveryId);
  }

  public void getDefaultDelivery(User u, DeliveryPrototype delivery) throws AdminException {
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
    if(u.isCreateArchive() && isArchiveDaemonDeployed()) {
      delivery.setArchiveTime(u.getDeliveryLifetime());
    }

    delivery.setUseDataSm(u.isUseDataSm());
    delivery.setTransactionMode(u.isTransactionMode());

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

    delivery.setMessageTimeToLive(u.getMessageTimeToLive());
  }

  public void copyUserSettingsToDeliveryPrototype(String user, DeliveryPrototype delivery) throws AdminException {
    User u = getUser(user);
    if (u == null) {
      throw new IntegrityException("user_not_exist", user);
    }
    getDefaultDelivery(u, delivery);
  }

  public void validateDeliveryWithIndividualTexts(DeliveryPrototype delivery) throws AdminException {
    DeliveryManager.validateDeliveryWithIndividualTexts(delivery);
  }

  public void validateDeliveryWithSingleText(DeliveryPrototype delivery) throws AdminException {
    DeliveryManager.validateDeliveryWithSingleText(delivery);
  }

  // RESTRICTIONS ================================================================================================================================

  public Restriction getRestriction(int id) {
    return restrictionProvider.getRestriction(id);
  }

  public List<Restriction> getRestrictions(RestrictionsFilter filter) {
    return restrictionProvider.getRestrictions(filter);
  }

  public synchronized void addRestriction(Restriction r) throws AdminException {
    restriction2UserDep.updateRestriction(r);
    restrictionProvider.addRestriction(r);
  }

  public synchronized void updateRestriction(Restriction r) throws AdminException {
    restriction2UserDep.updateRestriction(r);
    restrictionProvider.updateRestriction(r);
  }

  public synchronized void deleteRestriction(int id) throws AdminException {
    restrictionProvider.deleteRestriction(id);
  }

  // NOTIFICATIONS ==================================================================================================


  public NotificationSettings getNotificationSettings() {
    return webConfig.getNotificationSettings();
  }

  public void setNotificationSettings(NotificationSettings props) throws AdminException {
    deliveryNotificationsProvider.updateSettings(props);
    webConfig.setNotificationSettings(props);
  }

  public void sendTestSms(TestSms sms) throws AdminException {
    infosme.sendTestSms(sms);
  }

  public void sendTestEmailNotification(User user, String email, NotificationSettings settings) throws AdminException {
    deliveryNotificationsProvider.sendTestEmailNotification(user, email, settings);
  }

  public void sendTestSMSNotification(User user, Address address, DeliveryStatus status, NotificationSettings settings) throws AdminException {
    deliveryNotificationsProvider.sendTestSMSNotification(user,address,status, settings);
  }

  // CDR ==============================================================================================================

  public CdrSettings getCdrSettings() {
    return webConfig.getCdrSettings();
  }

  public void setCdrSettings(CdrSettings props) throws AdminException {
    webConfig.setCdrSettings(props);
    cdrProvider.updateSettings(props);
  }

  // SIEBEL =========================================================================================================

  public SiebelSettings getSiebelSettings() {
    return webConfig.getSiebelSettings();
  }

  public boolean checkSiebelSettings(SiebelSettings p) throws AdminException{
    siebelProvider.checkSettings(p);
    return true;
  }

  /**
   * Сохраняет настройки Siebel
   * @param siebelSettings настройик Siebel
   * @return стартовал ли SiebelManager на новых настройках
   * @throws AdminException ошибка валидации или сохранения
   */
  public synchronized boolean setSiebelSettings(SiebelSettings siebelSettings) throws AdminException {
    webConfig.setSiebelSettings(siebelSettings);
    return siebelProvider.updateSettings(siebelSettings);
  }

  // FILE DELIVERIES ==============================================================================================

  public void verifyCPSettings(User u, UserCPsettings ucps) throws AdminException {
    fileDeliveriesProvider.verifyConnection(u, ucps);
  }

  // ARCHIVE DAEMON =======================================================================================================

  public void startArchiveDaemon() throws AdminException {
    if(isArchiveDaemonDeployed()) {
      archiveDaemonManager.startArchiveDaemon();
    }
  }

  public void stopArchiveDaemon() throws AdminException {
    if(isArchiveDaemonDeployed()) {
      archiveDaemonManager.stopArchiveDaemon();
    }
  }

  public void switchArchiveDaemon(String toHost) throws AdminException {
    if(isArchiveDaemonDeployed()) {
      archiveDaemonManager.switchArchiveDaemon(toHost);
    }
  }

  public String getArchiveDaemonOnlineHost() throws AdminException {
    return isArchiveDaemonDeployed() ? archiveDaemonManager.getArchiveDaemonOnlineHost() : null;
  }

  public List<String> getArchiveDaemonHosts() throws AdminException {
    return isArchiveDaemonDeployed() ? archiveDaemonManager.getArchiveDaemonHosts() : null;
  }


  public void startPvss() throws AdminException {
    if(isPVSSDeployed()) {
      pvssManager.startPVSS();
    }
  }

  public void stopPvss() throws AdminException {
    if(isPVSSDeployed()) {
      pvssManager.stopPVSS();
    }
  }

  public List<String> getPvssHosts() throws AdminException {
    return isPVSSDeployed() ? pvssManager.getPVSSHosts() : null;
  }

  public String getPvssOnlineHosts() throws AdminException {
    return isPVSSDeployed() ? pvssManager.getPVSSOnlineHost() : null;
  }

  public void switchPvss(String toHost) throws AdminException {
    if(isPVSSDeployed()) {
      pvssManager.switchPVSS(toHost);
    }
  }

  public boolean isSmppGWDeployed() {
    return smppGWServiceManager != null;
  }


  public void startSmppGW() throws AdminException {
    if(isSmppGWDeployed()) {
      smppGWServiceManager.startSmppGW();
    }
  }

  public void stopSmppGW() throws AdminException {
    if(isSmppGWDeployed()) {
      smppGWServiceManager.stopSmppGW();
    }
  }

  public List<String> getSmppGWHosts() throws AdminException {
    return isSmppGWDeployed() ? smppGWServiceManager.getSmppGWHosts() : null;
  }

  public String getSmppGWOnlineHosts() throws AdminException {
    return isSmppGWDeployed() ? smppGWServiceManager.getSmppGWOnlineHost() : null;
  }

  public void switchSmppGW(String toHost) throws AdminException {
    if(isSmppGWDeployed()) {
      smppGWServiceManager.switchSmppGW(toHost);
    }
  }


  // ARCHIVE REQUESTS =====================================================================================================

  public DeliveriesRequest createRequest(String login, DeliveriesRequestPrototype request) throws AdminException {
    return archiveRequestsManager.createRequest(login, request);
  }

  public MessagesRequest createRequest(String login, MessagesRequestPrototype _request) throws AdminException {
    return archiveRequestsManager.createRequest(login, _request);
  }

  public Request getRequest(int requestId) throws AdminException {
    return archiveRequestsManager.getRequest(requestId);
  }

  public List<Request> getRequests(RequestFilter filter) throws AdminException {
    return archiveRequestsManager.getRequests(filter);
  }

  public void modifyRequest(Request request) throws AdminException {
    archiveRequestsManager.modifyRequest(request);
  }

  public void getDeliveriesResult(int requestId, Visitor<ArchiveDelivery> visitor) throws AdminException {
    archiveRequestsManager.getDeliveriesResult(requestId, visitor);
  }

  public void getMessagesResult(int requestId, Visitor<ArchiveMessage> visitor) throws AdminException {
    archiveRequestsManager.getMessagesResult(requestId, visitor);
  }

  public void removeRequest(int requestId) throws AdminException {
    archiveRequestsManager.removeRequest(requestId);
  }

  public void cancelRequest(int requestId) throws AdminException {
    archiveRequestsManager.cancelRequest(requestId);
  }

  public Delivery getArchiveDelivery(String login, int deliveryId) throws AdminException {
    User u = getUser(login);
    return archiveDeliveryManager.getDelivery(u.getLogin(), u.getPassword(), deliveryId);
  }

  public DeliveryStatistics getArchiveStatistics(String login, int deliveryId) throws AdminException {
    User u = getUser(login);
    return archiveDeliveryManager.getDeliveryStats(u.getLogin(), u.getPassword(), deliveryId);
  }

  public DeliveryStatusHistory getArchiveHistory(String login, int deliveryId) throws AdminException {
    User u = getUser(login);
    return archiveDeliveryManager.getDeliveryStatusHistory(u.getLogin(), u.getPassword(), deliveryId);

  }

  public void getArchiveMessages(String login, MessageFilter filter, int _pieceSize, Visitor<Message> visitor) throws AdminException {
    User u = getUser(login);
    archiveDeliveryManager.getMessages(login, u.getPassword(), filter, _pieceSize, visitor);

  }

  public int countArchiveMessages(String login, MessageFilter messageFilter) throws AdminException {
    User u = getUser(login);
    return archiveDeliveryManager.countMessages(login, u.getPassword(), messageFilter);
  }

  public void getMonitoringEvents(MonitoringEventsFilter eventsFilter, MonitoringJournal.Visitor v) throws AdminException{
    monitoringJournal.visit(eventsFilter,v);
  }

  public void resend(String login, int deliveryId, Collection<Long> messageIdsFilter, ResendListener listener) throws AdminException {
    User u = getUser(login);
    deliveryManager.resend(login, u.getPassword(), deliveryId, messageIdsFilter, listener);
  }

  public void resendAll(String login, int deliveryId, MessageFilter filter, ResendListener listener) throws AdminException {
    User u = getUser(login);
    deliveryManager.resendAll(login, u.getPassword(), deliveryId, filter, listener);
  }

  public SmppGWProviderSettings getSmppGWProviderSettings() {
    return smppGWManager.getProviderSettings();
  }

  public SmppGWEndpointSettings getSmppGWEndpointSettings() {
    return smppGWManager.getEndpointSettings();
  }

  public void updateSmppGWSettings(SmppGWEndpointSettings endpointSettings) throws AdminException {
    smppGWManager.updateSettings(endpointSettings);
  }

  public void updateSmppGWSettings(SmppGWProviderSettings providerSettings) throws AdminException {
    smppGWManager.updateSettings(providerSettings);
  }
}
package mobi.eyeline.informer.web.config;

import mobi.eyeline.informer.admin.AdminContext;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.archive.*;
import mobi.eyeline.informer.admin.cdr.CdrSettings;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.delivery.stat.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.informer.InformerSettings;
import mobi.eyeline.informer.admin.infosme.TestSms;
import mobi.eyeline.informer.admin.journal.Journal;
import mobi.eyeline.informer.admin.notifications.NotificationSettings;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.restriction.Restriction;
import mobi.eyeline.informer.admin.restriction.RestrictionsFilter;
import mobi.eyeline.informer.admin.siebel.SiebelSettings;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.DateAndFile;

import java.io.File;
import java.util.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Класс для управления конфигурациями
 *
 * @author Aleksandr Khalitov
 */
public class Configuration {

  private final Map<ConfigType, Revision> lastRevisions = new EnumMap<ConfigType, Revision>(ConfigType.class);

  private final Journal journal;

  private final AdminContext context;


  private InformerSettings informerSettings;

  public Configuration(AdminContext context) throws InitException {
    this.journal = context.getJournal();
    this.context = context;
    try {

      this.informerSettings = context.getConfigSettings();
    } catch (AdminException e) {
      throw new InitException(e);
    }
  }


  public InformerSettings getConfigSettings(){
    return informerSettings.cloneSettings();
  }

  private Revision buildNextRevision(String user, ConfigType type) {
    Revision revision = getLastRevision(type);
    if (revision == null) {
      revision = new Revision(user, 1);
    } else {
      revision = new Revision(user, revision.getNumber() + 1);
    }
    return revision;
  }

  public void setConfigSettings(InformerSettings newS, String user) throws AdminException {
    Revision revision = buildNextRevision(user, ConfigType.CONFIG);
    context.updateConfigSettings(newS);
    lastRevisions.put(ConfigType.CONFIG, revision);
    journal.logChanges(informerSettings, newS, user);
    informerSettings = context.getConfigSettings();
  }

  public Journal getJournal() {
    return journal;
  }

  public void addInBlacklist(Collection<Address> msisdns, String user) throws AdminException {
    context.addInBlacklist(msisdns);
    for (Address msisdn : msisdns) {
      journal.logAddBlacklist(msisdn.getSimpleAddress(), user);
    }
  }

  public boolean isBlackListEnabled() throws AdminException {
    return context.isBlackListEnabled();
  }

  public void addInBlacklist(Address msisdn, String user) throws AdminException {
    context.addInBlacklist(msisdn);
    journal.logAddBlacklist(msisdn.getSimpleAddress(), user);
  }

  public void removeFromBlacklist(Address msisdn, String user) throws AdminException {
    context.removeFromBlacklist(msisdn);
    journal.logRemoveBlacklist(msisdn.getSimpleAddress(), user);
  }

  public void removeFromBlacklist(Collection<Address> msisdns, String user) throws AdminException {
    context.removeFromBlacklist(msisdns);
    for (Address msisdn : msisdns) {
      journal.logRemoveBlacklist(msisdn.getSimpleAddress(), user);
    }
  }

  public boolean blacklistContains(Address msisdn) throws AdminException {
    return context.blacklistContains(msisdn);
  }

  public String getDefaultSmsc() {
    return context.getDefaultSmsc();
  }

  public void setDefaultSmsc(String newSmsc, String user) throws AdminException {
    String oldSmsc = context.getDefaultSmsc();
    if (!oldSmsc.equals(newSmsc)) {
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


  public void removeUser(String login, String user) throws AdminException {
    User u = context.getUser(login);
    if (u != null) {
      context.removeUser(u.getLogin());
      journal.logRemoveUser(u.getLogin(), user);
    }
  }

  public User getUser(String login) {
    return context.getUser(login);
  }

  public List<User> getUsers() {
    return context.getUsers();
  }

  public void updateUser(User u, String user) throws AdminException {
    User oldUser = context.getUser(u.getLogin());
    context.updateUser(u);
    journal.logUpdateUser(oldUser, u, user);
  }

  public void addUser(User u, String user) throws AdminException {
    context.addUser(u);
    journal.logAddUser(u.getLogin(), user);
  }

  public void updateRegionsConfiguration(Collection<Region> regions) throws AdminException {
    context.updateRegionsConfiguration(regions);
    //todo addrecord to journal
  }

  public void removeRegion(Integer regionId, String user) throws AdminException {
    Region r = getRegion(regionId);
    if (r != null) {
      context.removeRegion(r.getRegionId());
      journal.logRemoveRegion(r.getName(), user);
    }
  }

  public Region getRegion(Integer id) {
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

  public int getDefaultMaxPerSecond() {
    return context.getDefaultMaxPerSecond();
  }

  public void setDefaultMaxPerSecond(int defMaxPerSecond, String user) throws AdminException {
    int old = context.getDefaultMaxPerSecond();
    if (old != defMaxPerSecond) {
      context.setDefaultMaxPerSecond(defMaxPerSecond);
      journal.logSetDefaultSmsPerSecondRegion(old, defMaxPerSecond, user);
    }
  }

  public int countMessages(String login, String password, MessageFilter messageFilter) throws AdminException {
    return context.countMessages(login, password, messageFilter);
  }

  public Delivery getDelivery(String login, String password, int deliveryId) throws AdminException {
    return context.getDelivery(login, password, deliveryId);
  }

  public void pauseDelivery(String login, String password, int deliveryId) throws AdminException {
    context.pauseDelivery(login, password, deliveryId);
    journal.logDeliveryPaused(login, deliveryId);
  }

  public void activateDelivery(String login, String password, int deliveryId) throws AdminException {
    context.activateDelivery(login, password, deliveryId);
    journal.logDeliveryActivated(login, deliveryId);
  }

  public DeliveryStatistics getDeliveryStats(String login, String password, int deliveryId) throws AdminException {
    return context.getDeliveryStats(login, password, deliveryId);
  }

  public void getDeliveries(String login, String password, DeliveryFilter deliveryFilter, int _pieceSize, Visitor<Delivery> visitor) throws AdminException {
    context.getDeliveries(login, password, deliveryFilter, _pieceSize, visitor);
  }

  public void getMessagesStates(String login, String password, MessageFilter filter, int _pieceSize, Visitor<Message> visitor) throws AdminException {
    context.getMessagesStates(login, password, filter, _pieceSize, visitor);
  }

  public int countDeliveries(String login, String password, DeliveryFilter deliveryFilter) throws AdminException {
    return context.countDeliveries(login, password, deliveryFilter);
  }

  public void dropDelivery(String login, String password, int deliveryId) throws AdminException {
    Delivery oldD = context.getDelivery(login, password, deliveryId);
    if (oldD == null) {
      throw new DeliveryException("delivery_not_found");
    }
    context.dropDelivery(login, password, deliveryId);
    journal.logDeliveryDroped(login, oldD);
  }

  public void modifyDelivery(String login, String password, Delivery delivery) throws AdminException {
    Delivery oldD = context.getDelivery(login, password, delivery.getId());
    if (oldD == null) {
      throw new DeliveryException("delivery_not_found");
    }
    context.modifyDelivery(login, password, delivery);
    journal.logDeliveriesChanges(login, oldD, delivery);
  }

  public Delivery createDelivery(String login, String password, DeliveryPrototype delivery, DataSource<Message> msDataSource) throws AdminException {
    Delivery d = context.createDeliveryWithIndividualTexts(login, password, delivery, msDataSource);
    journal.logDeliveryCreated(login, d);
    return d;
  }

  public Delivery createSingleTextDelivery(String login, String password, DeliveryPrototype delivery, DataSource<Address> msDataSource) throws AdminException {
    Delivery d = context.createDeliveryWithSingleText(login, password, delivery, msDataSource);
    journal.logDeliveryCreated(login, d);
    return d;
  }

  public boolean isFtpServerDeployed() {
    return context.isFtpServerDeployed();
  }

  public boolean isArchiveDaemonDeployed() {
    return context.isArchiveDaemonDeployed();
  }

  public String getFtpServerOnlineHost() throws AdminException {
    return context.getFtpServerOnlineHost();
  }

  public List<String> getFtpServerHosts() throws AdminException {
    return context.getFtpServerHosts();
  }

  public void startFtpServer(String user) throws AdminException {
    context.startFtpServer();
    journal.logFtpServerStart(user);
  }

  public void stopFtpServer(String user) throws AdminException {
    context.stopFtpServer();
    journal.logFtpServerStop(user);
  }

  public void switchFtpServer(String toHost, String user) throws AdminException {
    context.switchFtpServer(toHost);
    journal.logFtpServerSwitch(toHost, user);
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

  public void startArchiveDaemon(String user) throws AdminException {
    context.startArchiveDaemon();
    journal.logArchiveDaemonStart(user);
  }

  public void stopArchiveDaemon(String user) throws AdminException {
    context.stopArchiveDaemon();
    journal.logArchiveDaemonStop(user);
  }

  public void switchArchiveDaemon(String toHost, String user) throws AdminException {
    context.switchArchiveDaemon(toHost);
    journal.logArchiveDaemonSwitch(toHost, user);
  }

  public String getArchiveDaemonOnlineHost() throws AdminException {
    return context.getArchiveDaemonOnlineHost();
  }

  public List<String> getArchiveDaemonHosts() throws AdminException {
    return context.getArchiveDaemonHosts();
  }

  public void statistics(DeliveryStatFilter filter, DeliveryStatVisitor visitor) throws AdminException {
    context.statistics(filter, visitor);
  }

  public void statisticByUsers(UserStatFilter filter, UserStatVisitor visitor) throws AdminException {
    context.statisticByUsers(filter, visitor);
  }

  public void dropStatEntities(Date from, Date till) throws AdminException {
    context.dropStatEntities(from, till);
  }

  public void getStatEntities(StatEntityProvider.EntityVisitor v, Date from, Date till) throws AdminException {
    context.getStatEntities(v, from, till);
  }

  public List<DateAndFile> getProcessedNotificationsFiles(Date startDate,Date endDate) throws AdminException {
    return context.getProcessedNotificationsFiles(startDate,endDate);
  }


  public DeliveryStatusHistory getDeliveryStatusHistory(String login, String password, int deliveryId) throws AdminException {
    return context.getDeliveryStatusHistory(login, password, deliveryId);
  }

  public Restriction getRestriction(int id) {
    return context.getRestriction(id);
  }

  public List<Restriction> getRestrictions(RestrictionsFilter filter) {
    return context.getRestrictions(filter);
  }

  public void addRestriction(Restriction r, String user) throws AdminException {
    context.addRestriction(r);
    journal.logAddRestriction(r, user);
  }

  public void updateRestriction(Restriction r, String user) throws AdminException {
    Restriction oldr = context.getRestriction(r.getId());
    context.updateRestriction(r);
    if (oldr != null)
      journal.logUpdateRestriction(r, oldr, user);
  }

  public void deleteRestriction(int id, String user) throws AdminException {
    Restriction oldr = context.getRestriction(id);
    context.deleteRestriction(id);
    journal.logDeleteRestriction(oldr, user);
  }

  public void addMessages(String login, String password, DataSource<Message> msDataSource, int deliveryId) throws AdminException {
    context.addMessages(login, password, msDataSource, deliveryId);
  }

  public void addSingleTextMessages(String login, String password, DataSource<Address> msDataSource, int deliveryId) throws AdminException {
    context.addSingleTextMessages(login, password, msDataSource, deliveryId);
  }

  public void copyUserSettingsToDeliveryPrototype(String user, DeliveryPrototype delivery) throws AdminException {
    context.copyUserSettingsToDeliveryPrototype(user, delivery);
  }

  public void sendTestSms(TestSms sms) throws AdminException {
    context.sendTestSms(sms);
  }


  public NotificationSettings getNotificationSettings() {
    return context.getNotificationSettings();
  }

  public void updateNotificationSettings(NotificationSettings props, String user) throws AdminException {
    NotificationSettings old = context.getNotificationSettings();
    context.setNotificationSettings(props);
    journal.logUpdateNotificationSettings(props, old, user);
  }


  public CdrSettings getCdrProperties() {
    return context.getCdrSettings();
  }

  public boolean checkSiebelSettings(SiebelSettings p) throws AdminException {
    return context.checkSiebelSettings(p);
  }

  public void setCdrSettings(CdrSettings props, String user) throws AdminException {
    CdrSettings old = context.getCdrSettings();
    context.setCdrSettings(props);
    journal.logUpdateCdrProps(props, old, user);
  }

  public boolean setSiebelSettings(SiebelSettings props, String user) throws AdminException {
    SiebelSettings old = context.getSiebelSettings();
    boolean res =  context.setSiebelSettings(props);
    journal.logUpdateSiebelProps(props, old, user);
    return res;
  }

  public SiebelSettings getSiebelSettings() {
    return context.getSiebelSettings();
  }

  public boolean isAllowUssdPushDeliveries() {
    return context.isAllowUssdPushDeliveries();
  }

  public void sendTestEmailNotification(User user, String email, NotificationSettings settings) throws AdminException {
    context.sendTestEmailNotification(user,email,settings);
  }

  public void sendTestSmsNotification(User user, Address address, DeliveryStatus status, NotificationSettings settings) throws AdminException {
    context.sendTestSMSNotification(user,address,status,settings);
  }

  private final Lock lock = new ReentrantLock();

  public void lock() {
    lock.lock();
  }

  public void unlock() {
    lock.unlock();
  }

  public FileSystem getFileSystem() {
    return context.getFileSystem();
  }

  public Revision getLastRevision(ConfigType configType) {
    return lastRevisions.get(configType);
  }

  public File getWorkDir() {
    return context.getWorkDir();
  }

  public void verifyCPSettings(User u, UserCPsettings ucps) throws AdminException {
    context.verifyCPSettings(u, ucps);
  }

  public void validateDeliveryWithSingleText(DeliveryPrototype delivery) throws AdminException {
    context.validateDeliveryWithSingleText(delivery);
  }

  public void validateDeliveryWithIndividualTexts(DeliveryPrototype delivery) throws AdminException {
    context.validateDeliveryWithIndividualTexts(delivery);
  }

  public void cancelRequest(int requestId) throws AdminException {
    context.cancelRequest(requestId);
  }

  public void removeRequest(int requestId) throws AdminException {
    context.removeRequest(requestId);
  }

  public void getMessagesResult(int requestId, Visitor<ArchiveMessage> visitor) throws AdminException {
    context.getMessagesResult(requestId, visitor);
  }

  public void getDeliveriesResult(int requestId, Visitor<ArchiveDelivery> visitor) throws AdminException {
    context.getDeliveriesResult(requestId, visitor);
  }

  public void modifyRequest(Request request) throws AdminException {
    context.modifyRequest(request);
  }

  public List<Request> getRequests() throws AdminException {
    return context.getRequests();
  }

  public Request getRequest(int requestId) throws AdminException {
    return context.getRequest(requestId);
  }

  public MessagesRequest createRequest(String login, MessagesRequestPrototype _request) throws AdminException {
    return context.createRequest(login, _request);
  }

  public DeliveriesRequest createRequest(String login, DeliveriesRequestPrototype request) throws AdminException {
    return context.createRequest(login, request);
  }

  public Delivery getArchiveDelivery(String login, int deliveryId) throws AdminException {
    return context.getArchiveDelivery(login, deliveryId);
  }

  public DeliveryStatusHistory getArchiveHistory(String login, int deliveryId) throws AdminException {
    return context.getArchiveHistory(login, deliveryId);
  }

  public DeliveryStatistics getArchiveStatistics(String login, int deliveryId) throws AdminException {
    return context.getArchiveStatistics(login, deliveryId);
  }

  public int countArchiveMessages(String login, MessageFilter messageFilter) throws AdminException {
    return context.countArchiveMessages(login, messageFilter);
  }

  public void getArchiveMessages(String login, MessageFilter filter, int _pieceSize, Visitor<Message> visitor) throws AdminException {
    context.getArchiveMessages(login, filter, _pieceSize, visitor);
  }

  public enum ConfigType {
    CONFIG, USERS
  }
}
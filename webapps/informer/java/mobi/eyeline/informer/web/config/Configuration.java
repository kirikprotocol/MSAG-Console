package mobi.eyeline.informer.web.config;

import mobi.eyeline.informer.admin.AdminContext;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.Daemon;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.informer.InformerSettings;
import mobi.eyeline.informer.admin.infosme.TestSms;
import mobi.eyeline.informer.admin.journal.Journal;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.restriction.Restriction;
import mobi.eyeline.informer.admin.restriction.RestrictionException;
import mobi.eyeline.informer.admin.restriction.RestrictionsFilter;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;

import java.io.File;
import java.util.*;
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



  private InformerSettings informerSettings;

  public Configuration(AdminContext context) throws InitException {
    this.journal = context.getJournal();
    this.context = context;
    try{

      this.informerSettings = context.getConfigSettings();
    }catch (AdminException e){
      throw new InitException(e);
    }
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


  public void removeUser(String login, String user) throws AdminException {
    User u = context.getUser(login);
    if(u != null) {
      context.removeUser(u.getLogin());
      journal.logRemoveUser(u.getLogin(), user);
    }
  }

  public User getUser(String login) {
    return context.getUser(login);
  }

  public List<User> getUsers()  {
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



  public void removeRegion(Integer regionId, String user) throws AdminException {
    Region r = getRegion(regionId);
    if(r != null) {
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
    if(old != defMaxPerSecond) {
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

  public void cancelDelivery(String login, String password, int deliveryId) throws AdminException {
    context.cancelDelivery(login, password, deliveryId);
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

  public void getDeliveries(String login, String password, DeliveryFilter deliveryFilter, int _pieceSize, Visitor<DeliveryInfo> visitor) throws AdminException {
    context.getDeliveries(login, password, deliveryFilter, _pieceSize, visitor);
  }

  public void getMessagesStates(String login, String password, MessageFilter filter, int _pieceSize, Visitor<MessageInfo> visitor) throws AdminException {
    context.getMessagesStates(login, password, filter, _pieceSize, visitor);
  }

  public int countDeliveries(String login, String password, DeliveryFilter deliveryFilter) throws AdminException {
    return context.countDeliveries(login, password, deliveryFilter);
  }

  public void dropDelivery(String login, String password, int deliveryId) throws AdminException {
    context.dropDelivery(login, password, deliveryId);
  }

  public void modifyDelivery(String login, String password, Delivery delivery) throws AdminException {
    Delivery oldD = context.getDelivery(login, password, delivery.getId());
    if(oldD == null) {
      throw new DeliveryException("delivery_not_found");
    }
    context.modifyDelivery(login, password, delivery);
    journal.logDeliveriesChanges(login, oldD, delivery);

  }

  public void createDelivery(String login, String password, Delivery delivery, DataSource<Message> msDataSource) throws AdminException {
    context.createDelivery(login, password, delivery, msDataSource);
  }

  public void createSingleTextDelivery(String login, String password, Delivery delivery, DataSource<Address> msDataSource) throws AdminException {
    context.createSingleTextDelivery(login, password, delivery, msDataSource);
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

  public List<Daemon> getDaemons(final String user) {
    List<Daemon> ds = context.getDaemons();
    List<Daemon> result = new ArrayList<Daemon>(ds.size());

    for(final Daemon d : ds) {
      result.add(new Daemon() {
        public String getName() {
          return d.getName();
        }
        public void start() throws AdminException {
          d.start();
          journal.logDaemonrStart(getName(), user);
        }
        public void stop() throws AdminException {
          d.stop();
          journal.logDaemonStop(getName(), user);
        }
        public boolean isStarted() {
          return d.isStarted();
        }
      });
    }

    return result;
  }

  public void statistics(DeliveryStatFilter filter, DeliveryStatVisitor visitor) throws AdminException{
    context.statistics(filter, visitor);
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
    journal.logAddRestriction(r,user);
  }

  public void updateRestriction(Restriction r, String user) throws AdminException {
    Restriction oldr = context.getRestriction(r.getId());
    if(oldr == null) {
      throw new RestrictionException("restriction.not.found");
    }
    context.updateRestriction(r);
    journal.logUpdateRestriction(r,oldr,user);
  }

  public void deleteRestriction(int id, String user) throws AdminException {
    Restriction oldr = context.getRestriction(id);
    context.deleteRestriction(id);
    journal.logDeleteRestriction(oldr,user);
  }

  public void addMessages(String login, String password, DataSource<Message> msDataSource, int deliveryId) throws AdminException {
    //todo journal
    context.addMessages(login, password, msDataSource, deliveryId);
  }

  public List<Long> addSingleTextMessages(String login, String password, DataSource<Address> msDataSource, int deliveryId) throws AdminException {
    //todo journal
    return context.addSingleTextMessages(login, password, msDataSource, deliveryId);
  }

  public void dropMessages(String login, String password, int deliveryId, long[] messageIds) throws AdminException {
    //todo journal
    context.dropMessages(login, password, deliveryId, messageIds);
  }

  public void sendTestSms(TestSms sms) throws AdminException {
    context.sendTestSms(sms);
  }

  public Properties getJavaMailProperties() {
    return context.getJavaMailProperties();
  }

  public void updateJavaMailProperties(Properties props,String user) throws AdminException {
    Properties old = context.getJavaMailProperties();
    context.setJavaMailProperties(props);
    journal.logUpdateJavaMailProps(props,old,user);
  }

  public Properties getNotificationTemplates() {
    return context.getNotificationTemplates();
  }

  public void updateNotificationTemplates(Properties props,String user) throws AdminException {
    Properties old = context.getNotificationTemplates();
    context.setNotificationTemplates(props);
    journal.logUpdateNotificationTemplates(props,old,user);
  }

  public Address getSmsSenderAddress() {
    return context.getSmsSenderAddress();
  }

  public void setSmsSenderAddress(Address addr, String user) throws AdminException {
    Address old = getSmsSenderAddress();
    context.setSmsSenderAddress(addr);
    journal.logUpdateSmsSenderAddress(addr,old,user);
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



  public File getWorkDir() {
    return context.getWorkDir();
  }

  public enum ConfigType {
    CONFIG, USERS
  }
}
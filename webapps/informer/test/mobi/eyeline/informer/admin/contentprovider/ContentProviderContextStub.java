package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;

import java.io.File;
import java.util.*;

/**
 * User: artem
 * Date: 08.07.11
 */
public class ContentProviderContextStub implements ContentProviderContext {

  private final FileSystem fs;
  private final Map<String, User> users;
  private final Map<String, Region> regions;
  private final TestDeliveryManager deliveryManager;
  private final Set<String> userRestrictions;

  private final Map<String, Integer> activateBans;
  private final Map<String, Integer> createBans;
  private final Map<String, Integer> addMessagesBans;

  public ContentProviderContextStub(FileSystem fs) {
    this.fs = fs;
    this.users = new HashMap<String, User>();
    this.regions = new HashMap<String, Region>();
    this.deliveryManager = new TestDeliveryManager(new File(""), fs);
    this.userRestrictions = new HashSet<String>();
    this.activateBans = new HashMap<String, Integer>();
    this.createBans = new HashMap<String, Integer>();
    this.addMessagesBans = new HashMap<String, Integer>();
  }

  public void deprecateDeliveryActivation(String deliveryName, int times) {
    activateBans.put(deliveryName, times);
  }

  public void deprecateDeliveryCreation(String deliveryName, int times) {
    createBans.put(deliveryName, times);
  }

  public void deprecateAddMessages(String deliveryName, int times) {
    addMessagesBans.put(deliveryName, times);
  }

  public void addUser(User user) {
    users.put(user.getLogin(), user);
  }

  public void addRegion(Region region, String... addresses) {
    for (String address : addresses)
      regions.put(address, region);
  }

  public void addRestriction(String login) {
    userRestrictions.add(login);
  }

  public void finalizeDelivery(int deliveryId) throws AdminException {
    deliveryManager.forceDeliveryFinalization(deliveryId);
  }

  @Override
  public FileSystem getFileSystem() {
    return fs;
  }

  @Override
  public List<User> getUsers() {
    return new ArrayList<User>(users.values());
  }

  @Override
  public User getUser(String login) {
    return users.get(login);
  }

  @Override
  public Region getRegion(Address ab) {
    return regions.get(ab.getSimpleAddress());
  }

  private void checkDeliveryCreationBan(String deliveryName) throws ContentProviderException {
    Integer times = createBans.get(deliveryName);
    if (times != null && times > 0) {
      createBans.put(deliveryName, times-1);
      throw new ContentProviderException("ioerror");
    }
  }

  private void checkAddMessagesBan(String deliveryName) throws ContentProviderException {
    Integer times = addMessagesBans.get(deliveryName);
    if (times != null && times > 0) {
      addMessagesBans.put(deliveryName, times-1);
      throw new ContentProviderException("ioerror");
    }
  }

  @Override
  public Delivery createDeliveryWithIndividualTexts(String login, DeliveryPrototype delivery, DataSource<Message> msDataSource) throws AdminException {
    checkDeliveryCreationBan(delivery.getName());
    return deliveryManager.createDeliveryWithIndividualTexts(login, "", delivery, msDataSource);
  }

  @Override
  public Delivery createDeliveryWithSingleTextWithData(String login, DeliveryPrototype delivery, DataSource<Message> msDataSource) throws AdminException {
    checkDeliveryCreationBan(delivery.getName());
    return deliveryManager.createDeliveryWithSingleTextWithData(login, "", delivery, msDataSource);
  }

  @Override
  public Delivery getDelivery(String login, int deliveryId) throws AdminException {
    return deliveryManager.getDelivery(login, "", deliveryId);
  }

  @Override
  public void activateDelivery(String login, int deliveryId) throws AdminException {
    Delivery d = getDelivery(login, deliveryId);
    Integer times = activateBans.get(d.getName());
    if (times != null && times > 0) {
      activateBans.put(d.getName(), times - 1);
      throw new ContentProviderException("ioerror");
    }

    deliveryManager.activateDelivery(login, "", deliveryId);
  }

  @Override
  public void copyUserSettingsToDeliveryPrototype(String login, DeliveryPrototype delivery) throws AdminException {
    User u = getUser(login);

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

  @Override
  public void addMessages(String login, DataSource<Message> messageSource, int deliveryId) throws AdminException {
    Delivery d = getDelivery(login, deliveryId);
    checkAddMessagesBan(d.getName());
    deliveryManager.addIndividualMessages(login, "", messageSource, deliveryId);
  }

  @Override
  public void addSingleMessagesWithData(String login, DataSource<Message> messageSource, int deliveryId) throws AdminException {
    Delivery d = getDelivery(login, deliveryId);
    checkAddMessagesBan(d.getName());
    deliveryManager.addSingleTextMessagesWithData(login, "", messageSource, deliveryId);
  }

  @Override
  public void getMessagesStates(String login, MessageFilter filter, int pieceSize, Visitor<Message> visitor) throws AdminException {
    deliveryManager.getMessages(login, "", filter, 1000, visitor);
  }

  @Override
  public File getFtpUserHomeDir(String login) throws AdminException {
    return new File(".");
  }

  @Override
  public void getDeliveries(String login, DeliveryFilter deliveryFilter, Visitor<Delivery> visitor) throws AdminException {
    deliveryManager.getDeliveries(login, "", deliveryFilter, 100, visitor);
  }

  @Override
  public void dropDelivery(String login, int deliveryId) throws AdminException {
    deliveryManager.dropDelivery(login, "", deliveryId);
  }

  @Override
  public void modifyDelivery(String login, Delivery delivery) throws AdminException {
    deliveryManager.modifyDelivery(login, "", delivery);
  }

  @Override
  public void checkNoRestrictions(String login) throws AdminException {
    if (userRestrictions.contains(login))
      throw new DeliveryException("User restricted");
  }
}

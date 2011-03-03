package mobi.eyeline.informer.admin.siebel;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangesDetector;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.filesystem.TestFileSystem;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Day;
import mobi.eyeline.informer.util.Time;

import java.util.ArrayList;
import java.util.List;

/**
* @author Aleksandr Khalitov
*/
class TestSiebelDeliveries implements SiebelContext {

  private User siebelUser;
  private final DeliveryChangesDetector detector;

  TestSiebelDeliveries(User siebelUser, DeliveryChangesDetector detector) {
    this.siebelUser = siebelUser;
    this.detector = detector;
  }

  private TestDeliveryManager deliveryManager = new TestDeliveryManager();


  public void addMessages(String login, DataSource<Message> msDataSource, int deliveryId) throws AdminException {
    deliveryManager.addIndividualMessages(login, siebelUser.getPassword(), msDataSource, deliveryId);
  }

  public Delivery createDeliveryWithIndividualTexts(String login, DeliveryPrototype delivery, DataSource<Message> msDataSource) throws AdminException {
    return deliveryManager.createDeliveryWithIndividualTexts(login, siebelUser.getPassword(), delivery, msDataSource);
  }

  public void dropDelivery(String login, int deliveryId) throws AdminException {
    deliveryManager.dropDelivery(login, siebelUser.getPassword(), deliveryId);
  }


  public Delivery getDelivery(String login, int deliveryId) throws AdminException {
    return deliveryManager.getDelivery(login, siebelUser.getPassword(), deliveryId);
  }

  public void modifyDelivery(String login, Delivery delivery) throws AdminException {
    deliveryManager.modifyDelivery(login, siebelUser.getPassword(), delivery);
  }

  public void cancelDelivery(String login, int deliveryId) throws AdminException {
    deliveryManager.cancelDelivery(login, siebelUser.getPassword(), deliveryId);
  }

  public void pauseDelivery(String login,  int deliveryId) throws AdminException {
    deliveryManager.pauseDelivery(login, siebelUser.getPassword(), deliveryId);
  }

  public void activateDelivery(String login, int deliveryId) throws AdminException {
    deliveryManager.activateDelivery(login, siebelUser.getPassword(), deliveryId);
  }


  public void getDeliveries(String login, DeliveryFilter deliveryFilter,  Visitor<Delivery> visitor) throws AdminException {
    deliveryManager.getDeliveries(login, siebelUser.getPassword(), deliveryFilter, 1000, visitor);
  }

  public void copyUserSettingsToDeliveryPrototype(String user, DeliveryPrototype delivery) throws AdminException {
    delivery.setOwner(siebelUser.getLogin());

    if (siebelUser.getSourceAddr() != null) {
      delivery.setSourceAddress(siebelUser.getSourceAddr());
    }
    if (siebelUser.getEmail() != null && siebelUser.isEmailNotification()) {
      delivery.setProperty(UserDataConsts.EMAIL_NOTIF_ADDRESS, siebelUser.getEmail());
    }
    if (siebelUser.getPhone() != null && siebelUser.isSmsNotification()) {
      delivery.setProperty(UserDataConsts.SMS_NOTIF_ADDRESS, siebelUser.getPhone().getSimpleAddress());
    }
    if(siebelUser.isCreateArchive()) {
      delivery.setArchiveTime(siebelUser.getDeliveryLifetime());
    }

    delivery.setUseDataSm(siebelUser.isUseDataSm());

    if (siebelUser.getDeliveryType() != null) {
      switch (siebelUser.getDeliveryType()) {
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
    if (siebelUser.isRetryOnFail()) {
      delivery.setRetryPolicy(siebelUser.getPolicyId());
      delivery.setRetryOnFail(true);
    }

    delivery.setPriority(siebelUser.getPriority());
    Time t;
    if((t = siebelUser.getDeliveryStartTime()) != null) {
      delivery.setActivePeriodStart(t);
    }

    if((t = siebelUser.getDeliveryEndTime()) != null) {
      delivery.setActivePeriodEnd(t);
    }

    delivery.setValidityPeriod(siebelUser.getValidityPeriod());
    if(siebelUser.getDeliveryDays() != null && !siebelUser.getDeliveryDays().isEmpty()) {
      List<Day> days = new ArrayList<Day>(7);
      for (Integer i : siebelUser.getDeliveryDays()) {
        days.add(Day.valueOf(i == 0 ? 7 : i));
      }
      delivery.setActiveWeekDays(days.toArray(new Day[days.size()]));
    }
  }

  @Override
  public Region getRegion(Address msisdn) throws AdminException {
    return null;
  }

  @Override
  public User getUser(String login) throws AdminException {
    return siebelUser;
  }

  @Override
  public DeliveryChangesDetector getDeliveryChangesDetector() {
    return detector;
  }

  @Override
  public FileSystem getFileSystem() {
    return new TestFileSystem();
  }

  public void shutdown() {
    deliveryManager.shutdown();
  }
}

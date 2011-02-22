package mobi.eyeline.informer.admin.siebel;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.TestAdminContext;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangesDetector;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;

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
    TestAdminContext.getDefaultDelivery(siebelUser, delivery);
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

  public void shutdown() {
    deliveryManager.shutdown();
  }
}

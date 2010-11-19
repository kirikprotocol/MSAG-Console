package mobi.eyeline.informer.admin.siebel;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.TestAdminContext;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.siebel.impl.SiebelDeliveries;
import mobi.eyeline.informer.admin.users.User;

/**
* @author Aleksandr Khalitov
*/
class TestSiebelDeliveries implements SiebelDeliveries {

  private User siebelUser;

  TestSiebelDeliveries(User siebelUser) {
    this.siebelUser = siebelUser;
  }

  private TestDeliveryManager deliveryManager = new TestDeliveryManager(null);

  public void addMessages(String login, String password, DataSource<Message> msDataSource, int deliveryId) throws AdminException {
    deliveryManager.addMessages(login, password, msDataSource, deliveryId);
  }
  public void createDelivery(String login, String password, Delivery delivery, DataSource<Message> msDataSource) throws AdminException {
    deliveryManager.createDelivery(login, password, delivery, msDataSource);
  }

  public void dropDelivery(String login, String password, int deliveryId) throws AdminException {
    deliveryManager.dropDelivery(login, password, deliveryId);
  }


  public Delivery getDelivery(String login, String password, int deliveryId) throws AdminException {
    return deliveryManager.getDelivery(login, password, deliveryId);
  }

  public void modifyDelivery(String login, String password, Delivery delivery) throws AdminException {
    deliveryManager.modifyDelivery(login, password, delivery);
  }

  public void cancelDelivery(String login, String password, int deliveryId) throws AdminException {
    deliveryManager.cancelDelivery(login, password, deliveryId);
  }

  public void pauseDelivery(String login, String password, int deliveryId) throws AdminException {
    deliveryManager.pauseDelivery(login, password, deliveryId);
  }

  public void activateDelivery(String login, String password, int deliveryId) throws AdminException {
    deliveryManager.activateDelivery(login, password, deliveryId);
  }


  public void getDeliveries(String login, String password, DeliveryFilter deliveryFilter, int _pieceSize, Visitor<DeliveryInfo> visitor) throws AdminException {
    deliveryManager.getDeliveries(login, password, deliveryFilter, _pieceSize, visitor);
  }

  public void getDefaultDelivery(String user, Delivery delivery) throws AdminException {
    TestAdminContext.getDefaultDelivery(siebelUser, delivery);
  }

  public void shutdown() {
    deliveryManager.shutdown();
  }
}

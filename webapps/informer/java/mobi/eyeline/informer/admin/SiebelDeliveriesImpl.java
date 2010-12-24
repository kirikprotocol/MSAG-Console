package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.siebel.impl.SiebelDeliveries;

/**
* @author Artem Snopkov
*/
class SiebelDeliveriesImpl implements SiebelDeliveries {

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

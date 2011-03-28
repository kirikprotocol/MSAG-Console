package mobi.eyeline.informer.admin.restriction;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryFilter;
import mobi.eyeline.informer.admin.delivery.Visitor;
import mobi.eyeline.informer.admin.users.User;

import java.util.Collections;
import java.util.List;

/**
 * User: artem
 * Date: 28.03.11
 */
class RestrictionContextStub implements RestrictionContext {
  @Override
  public List<User> getUsers() {
    return Collections.emptyList();
  }

  @Override
  public void getDeliveries(String login, DeliveryFilter dFilter, Visitor<Delivery> visitor) throws AdminException {
  }

  @Override
  public Delivery setDeliveryRestriction(String login, int deliveryId, boolean restriction) throws AdminException {
    return null;
  }

  @Override
  public void pauseDelivery(String login, int deliveryId) throws AdminException {
  }

  @Override
  public void activateDelivery(String login, int deliveryId) throws AdminException {
  }
}

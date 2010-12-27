package mobi.eyeline.informer.admin.restriction;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryFilter;
import mobi.eyeline.informer.admin.delivery.Visitor;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangesDetector;
import mobi.eyeline.informer.admin.users.User;

import java.util.List;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 02.12.2010
 * Time: 11:37:52
 */
public interface RestrictionContext {

  List<User> getUsers();

  void getDeliveries(String login, DeliveryFilter dFilter, Visitor<Delivery> visitor) throws AdminException;

  Delivery setDeliveryRestriction(String login, int deliveryId, boolean restriction) throws AdminException;

  void pauseDelivery(String login, int deliveryId) throws AdminException;

  void activateDelivery(String login, int deliveryId) throws AdminException;

}

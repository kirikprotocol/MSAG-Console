package mobi.eyeline.informer.admin.restriction;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryFilter;
import mobi.eyeline.informer.admin.delivery.Visitor;
import mobi.eyeline.informer.admin.users.User;

import java.util.List;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 02.12.2010
 * Time: 11:37:52
 */
public interface RestrictionDaemonContext {
  List<Restriction> getRestrictions(RestrictionsFilter rFilter);

  List<User> getUsers();


  void getDeliveries(String login, String password, DeliveryFilter dFilter, int piecesize, Visitor<Delivery> visitor) throws AdminException;

  void setDeliveryRestriction(String login, String password, Integer deliveryId, boolean restricted) throws AdminException;

  void pauseDelivery(String login, String password, Integer deliveryId) throws AdminException;

  void activateDelivery(String login, String password, Integer deliveryId) throws AdminException;
}

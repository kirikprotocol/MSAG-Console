package mobi.eyeline.informer.admin.restriction;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryFilter;
import mobi.eyeline.informer.admin.delivery.TestDeliveryManager;
import mobi.eyeline.informer.admin.delivery.Visitor;
import mobi.eyeline.informer.admin.users.TestUsersManager;
import mobi.eyeline.informer.admin.users.User;

import java.util.List;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 02.12.2010
 * Time: 12:30:20
 */
public class TestRestrictionsDaemonContext implements RestrictionDaemonContext {
  TestDeliveryManager deliveryManager;
  TestRestrictionsManager restrictionsManager;
  TestUsersManager usersManager;
  public TestRestrictionsDaemonContext(TestDeliveryManager deliveryManager, TestRestrictionsManager restrictionsManager, TestUsersManager usersManager) {
    this.deliveryManager=deliveryManager;
    this.restrictionsManager=restrictionsManager;
    this.usersManager=usersManager;
  }

  public List<Restriction> getRestrictions(RestrictionsFilter rFilter) {
    return restrictionsManager.getRestrictions(rFilter);
  }

  public List<User> getUsers() {
    return usersManager.getUsers();
  }

  public void getDeliveries(String login, String password, DeliveryFilter dFilter, int piecesize, Visitor<Delivery> visitor) throws AdminException {
    deliveryManager.getDeliveries(login,password,dFilter,piecesize,visitor);
  }

  public void setDeliveryRestriction(String login, String password, Integer deliveryId, boolean restricted) throws AdminException {
    Delivery d = deliveryManager.getDelivery(login, password, deliveryId);
    d.setProperty(UserDataConsts.RESTRICTION, Boolean.toString(restricted));
    deliveryManager.modifyDelivery(login, password, d);
  }

  public void pauseDelivery(String login, String password, Integer deliveryId) throws AdminException {
    deliveryManager.pauseDelivery(login,password,deliveryId);
  }

  public void activateDelivery(String login, String password, Integer deliveryId) throws AdminException {
    deliveryManager.activateDelivery(login,password,deliveryId);
  }
}

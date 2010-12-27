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
public class TestRestrictionsDaemonContext implements RestrictionContext {
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

  public void getDeliveries(String login, DeliveryFilter dFilter, Visitor<Delivery> visitor) throws AdminException {
    User u = usersManager.getUser(login);
    deliveryManager.getDeliveries(login, u.getPassword(), dFilter,1000,visitor);
  }

  public Delivery setDeliveryRestriction(String login, int deliveryId, boolean restricted) throws AdminException {
    User u = usersManager.getUser(login);
    String password = u.getPassword();
    Delivery d = deliveryManager.getDelivery(login, password, deliveryId);
    d.setProperty(UserDataConsts.RESTRICTION, Boolean.toString(restricted));
    deliveryManager.modifyDelivery(login, password, d);
    return d;
  }

  public void pauseDelivery(String login, int deliveryId) throws AdminException {
    User u = usersManager.getUser(login);
    String password = u.getPassword();
    deliveryManager.pauseDelivery(login,password,deliveryId);
  }

  public void activateDelivery(String login, int deliveryId) throws AdminException {
    User u = usersManager.getUser(login);
    String password = u.getPassword();
    deliveryManager.activateDelivery(login,password,deliveryId);
  }
}

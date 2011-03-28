package mobi.eyeline.informer.admin.restriction;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.users.User;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

/**
 * User: artem
 * Date: 28.03.11
 */
class RestrictionContextMock implements RestrictionContext {

  private Map<Integer, TestDelivery> deliveries = new HashMap<Integer, TestDelivery>();
  private List<User> users = new ArrayList<User>();
  private CountDownLatch pauseLatch, activateLatch;


  public void resetPauseLatch(int count) {
    pauseLatch = new CountDownLatch(count);
  }

  public void resetActivateLatch(int count) {
    activateLatch = new CountDownLatch(count);
  }

  public void waitPauseLatch(int timeout) throws InterruptedException {
    pauseLatch.await(timeout, TimeUnit.MILLISECONDS);
  }

  public void waitActivateLatch(int timeout) throws InterruptedException {
    activateLatch.await(timeout, TimeUnit.MILLISECONDS);
  }

  public void addDelivery(TestDelivery d) {
    deliveries.put(d.getId(), d);
  }

  public void addUser(User u) {
    users.add(u);
  }

  @Override
  public List<User> getUsers() {
    return users;
  }

  @Override
  public void getDeliveries(String login, DeliveryFilter dFilter, Visitor<Delivery> visitor) throws AdminException {
    for (Delivery d : deliveries.values()) {
      if (d.getOwner().equals(login)) {
        if (!visitor.visit(d))
          break;
      }
    }
  }

  @Override
  public Delivery setDeliveryRestriction(String login, int deliveryId, boolean restriction) throws AdminException {
    Delivery d = deliveries.get(deliveryId);
    if (d != null)
      d.setProperty(UserDataConsts.RESTRICTION, "true");
    return d;
  }

  @Override
  public void pauseDelivery(String login, int deliveryId) throws AdminException {
    TestDelivery d = deliveries.get(deliveryId);
    if (d != null)
      d.setStatus(DeliveryStatus.Paused);
    pauseLatch.countDown();
  }

  @Override
  public void activateDelivery(String login, int deliveryId) throws AdminException {
    TestDelivery d = deliveries.get(deliveryId);
    if (d != null)
      d.setStatus(DeliveryStatus.Active);
    activateLatch.countDown();
  }
}

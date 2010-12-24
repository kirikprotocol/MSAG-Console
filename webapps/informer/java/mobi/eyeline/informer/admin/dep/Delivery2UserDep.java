package mobi.eyeline.informer.admin.dep;

import mobi.eyeline.informer.admin.*;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UsersManager;

/**
 * @author Artem Snopkov
 */
public class Delivery2UserDep {

  private final DeliveryManager deliveryManager;
  private final UsersManager usersManager;

  public Delivery2UserDep(DeliveryManager deliveryManager, UsersManager usersManager) {
    this.deliveryManager = deliveryManager;
    this.usersManager = usersManager;
  }

  public void checkDelivery(DeliveryPrototype d) throws IntegrityException {
    User u = usersManager.getUser(d.getOwner());
    if (u == null) {
      throw new IntegrityException("user_not_exist", d.getOwner());
    }
  }

  public void checkDelivery(Delivery d) throws IntegrityException {
    User u = usersManager.getUser(d.getOwner());
    if (u == null) {
      throw new IntegrityException("user_not_exist", d.getOwner());
    }
  }

  public void removeUser(User user) throws AdminException {
    DeliveryFilter filter = new DeliveryFilter();
    filter.setUserIdFilter(user.getLogin());
    final String[] exist = new String[]{null};
    deliveryManager.getDeliveries(user.getLogin(), user.getPassword(), filter, 1, new Visitor<Delivery>() {
      public boolean visit(Delivery value) throws AdminException {
        exist[0] = value.getName();
        return false;
      }
    });
    if (exist[0] != null) {
      throw new IntegrityException("fail.delete.user.by.delivery", user.getLogin(), exist[0]);
    }
  }
}

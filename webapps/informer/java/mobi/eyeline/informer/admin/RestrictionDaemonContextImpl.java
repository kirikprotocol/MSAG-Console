package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryFilter;
import mobi.eyeline.informer.admin.delivery.Visitor;
import mobi.eyeline.informer.admin.restriction.Restriction;
import mobi.eyeline.informer.admin.restriction.RestrictionDaemonContext;
import mobi.eyeline.informer.admin.restriction.RestrictionsFilter;
import mobi.eyeline.informer.admin.users.User;

import java.util.List;

/**
* @author Artem Snopkov
*/
class RestrictionDaemonContextImpl implements RestrictionDaemonContext {
  private AdminContext adminContext;

  public RestrictionDaemonContextImpl(AdminContext adminContext) {
    this.adminContext = adminContext;
  }

  public List<Restriction> getRestrictions(RestrictionsFilter rFilter) {
    return adminContext.getRestrictions(rFilter);
  }

  public List<User> getUsers() {
    return adminContext.getUsers();
  }

  public void getDeliveries(String login, String password, DeliveryFilter dFilter, int piecesize, Visitor<Delivery> visitor) throws AdminException {
    adminContext.getDeliveries(login, password, dFilter, piecesize, visitor);
  }

  public void setDeliveryRestriction(String login, String password, Integer deliveryId, boolean restricted) throws AdminException {
    adminContext.setDeliveryRestriction(login, password, deliveryId, restricted);
  }

  public void restrictDelivery(String login, String password, Integer deliveryId) throws AdminException {
    synchronized (adminContext.getLock(deliveryId)) {
      setDeliveryRestriction(login, password, deliveryId, true);
      adminContext.deliveryManager.pauseDelivery(login, password, deliveryId);
    }
  }

  public void activateDelivery(String login, String password, Integer deliveryId) throws AdminException {
    adminContext.activateDelivery(login, password, deliveryId);
  }
}

package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.cdr.CdrDeliveries;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserException;

/**
* @author Artem Snopkov
*/
class CdrDeliveriesImpl implements CdrDeliveries {

  private AdminContext context;

  public CdrDeliveriesImpl(AdminContext context) {
    this.context = context;
  }

  public Delivery getDelviery(String user, int deliveryId) throws AdminException {
    User u = context.getUser(user);
    return context.getDelivery(user, u.getPassword(), deliveryId);
  }
}

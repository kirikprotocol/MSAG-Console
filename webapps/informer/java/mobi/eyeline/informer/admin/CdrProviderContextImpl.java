package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.cdr.CdrProviderContext;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangesDetector;
import mobi.eyeline.informer.admin.users.User;

/**
* @author Artem Snopkov
*/
class CdrProviderContextImpl implements CdrProviderContext {

  private AdminContext context;
  private DeliveryChangesDetector detector;

  public CdrProviderContextImpl(AdminContext context, DeliveryChangesDetector detector) {
    this.context = context;
    this.detector = detector;
  }

  public Delivery getDelviery(String user, int deliveryId) throws AdminException {
    User u = context.getUser(user);
    return context.getDelivery(user, u.getPassword(), deliveryId);
  }

  @Override
  public User getUser(String login) {
    return context.getUser(login);
  }

  @Override
  public DeliveryChangesDetector getDeliveryChangesDetector() {
    return detector;
  }
}

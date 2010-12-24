package mobi.eyeline.informer.admin.cdr;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangesDetector;
import mobi.eyeline.informer.admin.users.User;

/**
 * @author Artem Snopkov
 */
public interface CdrProviderContext {

  public Delivery getDelivery(String user, int deliveryId) throws AdminException;

  public User getUser(String login);

  public DeliveryChangesDetector getDeliveryChangesDetector();
}

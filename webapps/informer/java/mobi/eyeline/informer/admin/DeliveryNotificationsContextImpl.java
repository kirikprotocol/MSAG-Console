package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.infosme.TestSms;
import mobi.eyeline.informer.admin.notifications.DeliveryNotificationsContext;
import mobi.eyeline.informer.admin.notifications.NotificationSettings;
import mobi.eyeline.informer.admin.users.User;

/**
* @author Artem Snopkov
*/
class DeliveryNotificationsContextImpl implements DeliveryNotificationsContext {
  private AdminContext context;

  DeliveryNotificationsContextImpl(AdminContext context) {
    this.context = context;
  }

  public User getUser(String userId) throws AdminException {
    return context.getUser(userId);
  }

  public Delivery getDelivery(String login, String password, int deliveryId) throws AdminException {
    return context.getDelivery(login, password, deliveryId);
  }

  public NotificationSettings getNotificationSettings() {
    return context.getNotificationSettings();
  }

  public void sendTestSms(TestSms testSms) throws AdminException {
    context.sendTestSms(testSms);
  }
}

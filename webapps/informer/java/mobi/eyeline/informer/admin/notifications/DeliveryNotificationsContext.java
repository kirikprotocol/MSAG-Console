package mobi.eyeline.informer.admin.notifications;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.infosme.TestSms;
import mobi.eyeline.informer.admin.users.User;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 25.11.2010
 * Time: 15:51:54
 */
public interface DeliveryNotificationsContext {

  User getUser(String userId) throws AdminException;

  Delivery getDelivery(String login, int deliveryId) throws AdminException;

  NotificationSettings getNotificationSettings();

  void sendTestSms(TestSms testSms) throws AdminException;


}

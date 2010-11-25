package mobi.eyeline.informer.admin.notifications;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.infosme.TestSms;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;

import java.lang.reflect.Field;
import java.util.Properties;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 25.11.2010
 * Time: 15:51:54
 */
public interface DeliveryNotificationsContext {

  User getUser(String userId) throws AdminException;

  Delivery getDelivery(String login, String password, int deliveryId) throws AdminException;

  Address getSmsSenderAddress() throws AdminException;

  Properties getNotificationTemplates() ;

  void sendTestSms(TestSms testSms) throws AdminException;

  Properties getJavaMailProperties();


}

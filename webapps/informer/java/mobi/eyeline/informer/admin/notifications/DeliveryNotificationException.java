package mobi.eyeline.informer.admin.notifications;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 02.12.2010
 * Time: 14:16:03
 */
public class DeliveryNotificationException extends AdminException {
  DeliveryNotificationException(String key, Throwable cause, String... args) {
    super(key, cause, args);
  }

  DeliveryNotificationException(String key, String... args) {
    super(key, args);
  }
}

package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 12.11.2010
 * Time: 18:43:59
 */
public interface DeliveryNotificationsListener {

  public void onDeliveryNotification(DeliveryNotification notification);
  
}

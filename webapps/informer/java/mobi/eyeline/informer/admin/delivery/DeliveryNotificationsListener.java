package mobi.eyeline.informer.admin.delivery;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 12.11.2010
 * Time: 18:43:59
 */
public interface DeliveryNotificationsListener {

  public void onDeliveryStartNotification(DeliveryNotification notification)  throws Exception;
  public void onDeliveryFinishNotification(DeliveryNotification notification) throws Exception;
  public void onMessageNotification(DeliveryMessageNotification notification) throws Exception;

}

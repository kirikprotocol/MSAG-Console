package mobi.eyeline.informer.admin.delivery;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 23.11.2010
 * Time: 11:30:58
 */
// todo Причем тут адаптер? Адаптер приводит один интерфейс к другому, а здесь просто пустая реализация интерфейса.
public class DeliveryNotificationsAdapter implements DeliveryNotificationsListener{

  public void onDeliveryStartNotification(DeliveryNotification notification) throws Exception {

  }

  public void onDeliveryFinishNotification(DeliveryNotification notification) throws Exception {

  }

  public void onMessageNotification(DeliveryMessageNotification notification) throws Exception {

  }
}
